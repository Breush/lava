#include "./deep-deferred-stage.hpp"

#include "../../helpers/frustum.hpp"
#include "../cameras/i-camera-impl.hpp"
#include "../helpers/format.hpp"
#include "../lights/i-light-impl.hpp"
#include "../meshes/i-mesh-impl.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../vertex.hpp"

using namespace lava::magma;
using namespace lava::chamber;

DeepDeferredStage::DeepDeferredStage(RenderScene::Impl& scene)
    : m_scene(scene)
    , m_renderPassHolder(m_scene.engine())
    , m_clearPipelineHolder(m_scene.engine())
    , m_geometryPipelineHolder(m_scene.engine())
    , m_epiphanyPipelineHolder(m_scene.engine())
    , m_gBufferInputDescriptorHolder(m_scene.engine())
    , m_gBufferSsboDescriptorHolder(m_scene.engine())
    , m_gBufferSsboHeaderBufferHolder(m_scene.engine())
    , m_gBufferSsboListBufferHolder(m_scene.engine())
    , m_finalImageHolder(m_scene.engine(), "magma.vulkan.stages.deep-deferred-stage.final-image")
    , m_depthImageHolder(m_scene.engine(), "magma.vulkan.stages.deep-deferred-stage.depth-image")
    , m_framebuffer(m_scene.engine().device())
{
}

void DeepDeferredStage::init(uint32_t cameraId)
{
    m_cameraId = cameraId;

    logger.info("magma.vulkan.stages.deep-deferred-stage") << "Initializing." << std::endl;
    logger.log().tab(1);

    initGBuffer();
    initClearPass();
    initGeometryPass();
    initEpiphanyPass();

    //----- Render pass

    m_renderPassHolder.add(m_clearPipelineHolder);
    m_renderPassHolder.add(m_geometryPipelineHolder);
    m_renderPassHolder.add(m_epiphanyPipelineHolder);
    m_renderPassHolder.init();

    logger.log().tab(-1);
}

void DeepDeferredStage::update(vk::Extent2D extent, vk::PolygonMode polygonMode)
{
    m_extent = extent;

    m_clearPipelineHolder.update(extent);
    m_geometryPipelineHolder.update(extent, polygonMode);
    m_epiphanyPipelineHolder.update(extent);

    createResources();
    createFramebuffers();
}

void DeepDeferredStage::render(vk::CommandBuffer commandBuffer)
{
    const auto& deviceHolder = m_scene.engine().deviceHolder();
    deviceHolder.debugBeginRegion(commandBuffer, "deep-deferred");

    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 8> clearValues;

    // @note This clear color is used to reset gBufferRenderTargets[0].y to zero,
    // meaning that there is no opaque material there. The effective clear color
    // is currently hard-coded (@fixme) in epiphany.frag.
    clearValues[0].color = vk::ClearColorValue(std::array<float, 4u>{0.f, 0.f, 0.f, 0.f});
    clearValues[DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT].depthStencil = vk::ClearDepthStencilValue{1.f, 0u};

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPassHolder.renderPass();
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    //----- Clear pass

    deviceHolder.debugBeginRegion(commandBuffer, "deep-deferred.clear");

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_clearPipelineHolder.pipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_clearPipelineHolder.pipelineLayout(),
                                     DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX, 1, &m_gBufferSsboDescriptorSet, 0, nullptr);
    commandBuffer.draw(6, 1, 0, 0);

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Geometry pass

    deviceHolder.debugBeginRegion(commandBuffer, "deep-deferred.geometry");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_geometryPipelineHolder.pipeline());

    // Set the camera
    auto& camera = m_scene.camera(m_cameraId);
    camera.render(commandBuffer, m_geometryPipelineHolder.pipelineLayout(), CAMERA_DESCRIPTOR_SET_INDEX);
    const auto& cameraFrustum = camera.frustum();

    // Draw all meshes
    for (auto& mesh : m_scene.meshes()) {
        const auto& boundingSphere = mesh->interfaceImpl().boundingSphere();
        if (!camera.useFrustumCulling() || helpers::isVisibleInsideFrustum(boundingSphere, cameraFrustum)) {
            tracker.counter("draw-calls.renderer") += 1u;
            mesh->interfaceImpl().render(commandBuffer, m_geometryPipelineHolder.pipelineLayout(),
                                         GEOMETRY_MESH_DESCRIPTOR_SET_INDEX, GEOMETRY_MATERIAL_DESCRIPTOR_SET_INDEX);
        }
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Epiphany pass

    deviceHolder.debugBeginRegion(commandBuffer, "deep-deferred.epiphany");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_epiphanyPipelineHolder.pipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_epiphanyPipelineHolder.pipelineLayout(),
                                     DEEP_DEFERRED_GBUFFER_INPUT_DESCRIPTOR_SET_INDEX, 1, &m_gBufferInputDescriptorSet, 0,
                                     nullptr);

    // Bind lights
    for (auto lightId = 0u; lightId < m_scene.lightsCount(); ++lightId) {
        m_scene.light(lightId).render(commandBuffer, m_epiphanyPipelineHolder.pipelineLayout(), LIGHTS_DESCRIPTOR_SET_INDEX);
    }

    commandBuffer.draw(6, 1, 0, 1);

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Epilogue

    commandBuffer.endRenderPass();

    deviceHolder.debugEndRegion(commandBuffer);
}

RenderImage DeepDeferredStage::renderImage() const
{
    return m_finalImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA + m_cameraId);
}

RenderImage DeepDeferredStage::depthRenderImage() const
{
    return m_depthImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA_DEPTH + m_cameraId);
}

void DeepDeferredStage::changeRenderImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    m_finalImageHolder.changeLayout(imageLayout, commandBuffer);
}

//----- Internal

void DeepDeferredStage::initGBuffer()
{
    std::vector<uint32_t> inputAttachmentSizes(DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT, 1u);

    m_gBufferInputDescriptorHolder.inputAttachmentSizes(inputAttachmentSizes);
    m_gBufferInputDescriptorHolder.init(1, vk::ShaderStageFlagBits::eFragment);
    m_gBufferInputDescriptorSet = m_gBufferInputDescriptorHolder.allocateSet("deep-deferred.g-buffer-input");

    m_gBufferSsboDescriptorHolder.storageBufferSizes({1, 1});
    m_gBufferSsboDescriptorHolder.init(1, vk::ShaderStageFlagBits::eFragment);
    m_gBufferSsboDescriptorSet = m_gBufferSsboDescriptorHolder.allocateSet("deep-deferred.g-buffer-ssbo");
}

void DeepDeferredStage::initClearPass()
{
    //----- Shaders

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/fullscreen.vert");
    m_clearPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH"] = std::to_string(DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT);
    moduleOptions.defines["G_BUFFER_DATA_SIZE"] = std::to_string(G_BUFFER_DATA_SIZE);
    auto fragmentShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/renderers/deep-deferred/clear.frag", moduleOptions);
    m_clearPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    //----- Descriptor set layouts

    m_clearPipelineHolder.add(m_gBufferInputDescriptorHolder.setLayout());
    m_clearPipelineHolder.add(m_gBufferSsboDescriptorHolder.setLayout());
}

void DeepDeferredStage::initGeometryPass()
{
    //----- Shaders

    updateGeometryPassShaders(true);

    //----- Descriptor set layouts

    // @note Ordering is important
    m_geometryPipelineHolder.add(m_gBufferInputDescriptorHolder.setLayout());
    m_geometryPipelineHolder.add(m_gBufferSsboDescriptorHolder.setLayout());
    m_geometryPipelineHolder.add(m_scene.cameraDescriptorHolder().setLayout());
    m_geometryPipelineHolder.add(m_scene.materialDescriptorHolder().setLayout());
    m_geometryPipelineHolder.add(m_scene.meshDescriptorHolder().setLayout());

    //----- Rasterization

    m_geometryPipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().physicalDevice());
    m_geometryPipelineHolder.set(depthStencilAttachment);

    vulkan::PipelineHolder::ColorAttachment gBufferNodeColorAttachment;
    gBufferNodeColorAttachment.format = vk::Format::eR32G32B32A32Uint;
    gBufferNodeColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // There are multiple nodes render targets
    for (auto i = 0u; i < DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT; ++i) {
        m_geometryPipelineHolder.add(gBufferNodeColorAttachment);
    }

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(vulkan::Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(vulkan::Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(vulkan::Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(vulkan::Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(vulkan::Vertex, tangent)}};
    m_geometryPipelineHolder.set(vertexInput);
}

void DeepDeferredStage::initEpiphanyPass()
{
    //----- Shaders

    updateEpiphanyPassShaders(true);

    //----- Descriptors

    m_epiphanyPipelineHolder.add(m_gBufferInputDescriptorHolder.setLayout());
    m_epiphanyPipelineHolder.add(m_gBufferSsboDescriptorHolder.setLayout());
    m_epiphanyPipelineHolder.add(m_scene.cameraDescriptorHolder().setLayout());
    m_epiphanyPipelineHolder.add(m_scene.lightsDescriptorHolder().setLayout());

    //----- Attachments

    vulkan::PipelineHolder::InputAttachment gBufferInputNodeAttachment;
    gBufferInputNodeAttachment.format = vk::Format::eR32G32B32A32Uint;

    // There are input render targets
    for (auto i = 0u; i < DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT; ++i) {
        m_epiphanyPipelineHolder.add(gBufferInputNodeAttachment);
    }

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    m_epiphanyPipelineHolder.add(finalColorAttachment);
}

void DeepDeferredStage::updateGeometryPassShaders(bool firstTime)
{
    m_geometryPipelineHolder.removeShaderStages();

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH"] = std::to_string(DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT);
    moduleOptions.defines["CAMERA_DESCRIPTOR_SET_INDEX"] = std::to_string(CAMERA_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["MESH_DESCRIPTOR_SET_INDEX"] = std::to_string(GEOMETRY_MESH_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["MATERIAL_DESCRIPTOR_SET_INDEX"] = std::to_string(GEOMETRY_MATERIAL_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["G_BUFFER_DATA_SIZE"] = std::to_string(G_BUFFER_DATA_SIZE);
    if (firstTime) moduleOptions.updateCallback = [this]() { updateGeometryPassShaders(false); };

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/geometry.vert", moduleOptions);
    m_geometryPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});
    auto fragmentShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/renderers/deep-deferred/geometry.frag", moduleOptions);
    m_geometryPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    if (!firstTime) {
        m_geometryPipelineHolder.update(m_extent);
    }
}

void DeepDeferredStage::updateEpiphanyPassShaders(bool firstTime)
{
    m_epiphanyPipelineHolder.removeShaderStages();

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH"] = std::to_string(DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_INPUT_DESCRIPTOR_SET_INDEX"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_INPUT_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT);
    moduleOptions.defines["CAMERA_DESCRIPTOR_SET_INDEX"] = std::to_string(CAMERA_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["LIGHTS_DESCRIPTOR_SET_INDEX"] = std::to_string(LIGHTS_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["LIGHT_TYPE_POINT"] = std::to_string(static_cast<uint32_t>(LightType::Point));
    moduleOptions.defines["LIGHT_TYPE_DIRECTIONAL"] = std::to_string(static_cast<uint32_t>(LightType::Directional));
    moduleOptions.defines["G_BUFFER_DATA_SIZE"] = std::to_string(G_BUFFER_DATA_SIZE);
    if (firstTime) moduleOptions.updateCallback = [this]() { updateEpiphanyPassShaders(false); };

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/fullscreen.vert");
    m_epiphanyPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    auto fragmentShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/renderers/deep-deferred/epiphany.frag", moduleOptions);
    m_epiphanyPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    if (!firstTime) {
        m_epiphanyPipelineHolder.update(m_extent);
    }
}

void DeepDeferredStage::createResources()
{
    // GBuffer Input
    m_gBufferInputNodeImageHolders.clear();
    m_gBufferInputNodeImageHolders.reserve(DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT);
    auto gBufferHeaderFormat = vk::Format::eR32G32B32A32Uint;
    for (auto i = 0u; i < DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT; ++i) {
        m_gBufferInputNodeImageHolders.emplace_back(m_scene.engine());
        m_gBufferInputNodeImageHolders[i].create(gBufferHeaderFormat, m_extent, vk::ImageAspectFlagBits::eColor);
        m_gBufferInputDescriptorHolder.updateSet(m_gBufferInputDescriptorSet, m_gBufferInputNodeImageHolders[i].view(),
                                                 vk::ImageLayout::eShaderReadOnlyOptimal, i);
    }

    // GBuffer SSBO
    vk::DeviceSize headerSize = 1u * sizeof(uint32_t) + m_extent.width * m_extent.height * sizeof(uint32_t);
    m_gBufferSsboHeaderBufferHolder.create(vk::BufferUsageFlagBits::eStorageBuffer, headerSize);
    m_gBufferSsboDescriptorHolder.updateSet(m_gBufferSsboDescriptorSet, m_gBufferSsboHeaderBufferHolder.buffer(), headerSize, 0);
    m_gBufferSsboHeaderBufferHolder.copy(m_extent.width);

    vk::DeviceSize listSize =
        1u * sizeof(uint32_t) + DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH * m_extent.width * m_extent.height * sizeof(GBufferNode);
    m_gBufferSsboListBufferHolder.create(vk::BufferUsageFlagBits::eStorageBuffer, listSize);
    m_gBufferSsboDescriptorHolder.updateSet(m_gBufferSsboDescriptorSet, m_gBufferSsboListBufferHolder.buffer(), listSize, 1);

    logger.info("magma.vulkan.stages.deep-deferred-stage")
        << "GBuffer sizes | header: " << headerSize / 1000000.f << "Mo | list: " << listSize / 1000000.f << "Mo." << std::endl;

    // Final
    auto finalFormat = vk::Format::eR8G8B8A8Unorm;
    m_finalImageHolder.create(finalFormat, m_extent, vk::ImageAspectFlagBits::eColor);

    // Depth
    auto depthFormat = vulkan::depthBufferFormat(m_scene.engine().physicalDevice());
    m_depthImageHolder.create(depthFormat, m_extent, vk::ImageAspectFlagBits::eDepth);
}

void DeepDeferredStage::createFramebuffers()
{
    // Attachments
    std::vector<vk::ImageView> attachments;
    attachments.reserve(2u * DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT + 2u);

    for (auto i = 0u; i < DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT; ++i) {
        attachments.emplace_back(m_gBufferInputNodeImageHolders[i].view());
    }

    attachments.emplace_back(m_depthImageHolder.view());
    attachments.emplace_back(m_finalImageHolder.view());

    for (auto i = 0u; i < DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT; ++i) {
        attachments.emplace_back(m_gBufferInputNodeImageHolders[i].view());
    }

    // Framebuffer
    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPassHolder.renderPass();
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    if (m_scene.engine().device().createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.deep-deferred-stage") << "Failed to create framebuffers." << std::endl;
    }
}
