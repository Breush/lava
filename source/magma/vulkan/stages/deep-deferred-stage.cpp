#include "./deep-deferred-stage.hpp"

#include <lava/chamber/logger.hpp>

#include "../cameras/i-camera-impl.hpp"
#include "../lights/i-light-impl.hpp"
#include "../lights/point-light-impl.hpp"
#include "../meshes/i-mesh-impl.hpp"
#include "../render-engine-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../vertex.hpp"

namespace {
    constexpr const auto GBUFFER_MAX_NODE_DEPTH = 3u;
    constexpr const auto GBUFFER_MAX_NODE_DEPTH_STRING = "3";

    struct EpiphanyPointLightUbo {
        glm::vec4 wPosition;
        float radius;
    };

    vk::Format findSupportedFormat(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& candidates,
                                   vk::ImageTiling tiling, vk::FormatFeatureFlags features)
    {
        for (auto format : candidates) {
            vk::FormatProperties props = physicalDevice.getFormatProperties(format);

            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        return vk::Format::eUndefined;
    }

    vk::Format findDepthBufferFormat(vk::PhysicalDevice physicalDevice)
    {
        return findSupportedFormat(physicalDevice,
                                   {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                   vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }
}

using namespace lava::magma;
using namespace lava::chamber;

DeepDeferredStage::DeepDeferredStage(RenderScene::Impl& scene)
    : m_scene(scene)
    , m_renderPassHolder(m_scene.engine())
    , m_clearPipelineHolder(m_scene.engine())
    , m_geometryPipelineHolder(m_scene.engine())
    , m_epiphanyPipelineHolder(m_scene.engine())
    , m_lightsDescriptorHolder(m_scene.engine())
    , m_lightsUboHolder(m_scene.engine())
    , m_gBufferInputDescriptorHolder(m_scene.engine()) 
    , m_gBufferSsboDescriptorHolder(m_scene.engine())
    , m_gBufferSsboHeaderBufferHolder(m_scene.engine())
    , m_gBufferSsboListBufferHolder(m_scene.engine())
    , m_finalImageHolder(m_scene.engine())
    , m_depthImageHolder(m_scene.engine())
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

void DeepDeferredStage::update(vk::Extent2D extent)
{
    m_extent = extent;

    m_clearPipelineHolder.update(extent);
    m_geometryPipelineHolder.update(extent);
    m_epiphanyPipelineHolder.update(extent);

    createResources();
    createFramebuffers();
}

void DeepDeferredStage::render(vk::CommandBuffer commandBuffer)
{
    const auto& deviceHolder = m_scene.engine().deviceHolder();
    deviceHolder.debugMarkerBeginRegion(commandBuffer, "DeepDeferredStage");

    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 8> clearValues;
    clearValues[3].depthStencil = vk::ClearDepthStencilValue{1.f, 0u};

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPassHolder.renderPass();
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    //----- Clear pass

    deviceHolder.debugMarkerBeginRegion(commandBuffer, "Clear pass");

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_clearPipelineHolder.pipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_clearPipelineHolder.pipelineLayout(),
                                     GBUFFER_LIST_DESCRIPTOR_SET_INDEX, 1, &m_gBufferSsboDescriptorSet, 0, nullptr);
    commandBuffer.draw(6, 1, 0, 0);

    deviceHolder.debugMarkerEndRegion(commandBuffer);

    //----- Geometry pass

    deviceHolder.debugMarkerBeginRegion(commandBuffer, "Geometry pass");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_geometryPipelineHolder.pipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_epiphanyPipelineHolder.pipelineLayout(),
                                     LIGHTS_DESCRIPTOR_SET_INDEX, 1, &m_lightsDescriptorSet, 0, nullptr);

    // Set the camera
    auto& camera = m_scene.camera(m_cameraId);
    camera.render(commandBuffer, m_geometryPipelineHolder.pipelineLayout(), CAMERA_DESCRIPTOR_SET_INDEX);

    // Draw all meshes
    for (auto& mesh : m_scene.meshes()) {
        mesh->interfaceImpl().render(commandBuffer, m_geometryPipelineHolder.pipelineLayout(), GEOMETRY_MESH_DESCRIPTOR_SET_INDEX,
                                     GEOMETRY_MATERIAL_DESCRIPTOR_SET_INDEX);
    }

    deviceHolder.debugMarkerEndRegion(commandBuffer);

    //----- Epiphany pass

    deviceHolder.debugMarkerBeginRegion(commandBuffer, "Epiphany pass");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_epiphanyPipelineHolder.pipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_epiphanyPipelineHolder.pipelineLayout(),
                                     GBUFFER_HEADER_DESCRIPTOR_SET_INDEX, 1, &m_gBufferInputDescriptorSet, 0, nullptr);

    // @todo With LLL subpass, this shouldn't be needed anymore
    updateEpiphanyUbo();

    commandBuffer.draw(6, 1, 0, 1);

    deviceHolder.debugMarkerEndRegion(commandBuffer);

    //----- Epilogue

    commandBuffer.endRenderPass();

    deviceHolder.debugMarkerEndRegion(commandBuffer);
}

//----- Internal

void DeepDeferredStage::initGBuffer()
{
    m_gBufferInputDescriptorHolder.inputAttachmentSizes({1, 1, 1});
    m_gBufferInputDescriptorHolder.init(1, vk::ShaderStageFlagBits::eFragment);
    m_gBufferInputDescriptorSet = m_gBufferInputDescriptorHolder.allocateSet();

    m_gBufferSsboDescriptorHolder.storageBufferSizes({1, 1});
    m_gBufferSsboDescriptorHolder.init(1, vk::ShaderStageFlagBits::eFragment);
    m_gBufferSsboDescriptorSet = m_gBufferSsboDescriptorHolder.allocateSet();

    m_lightsDescriptorHolder.uniformBufferSizes({1});
    m_lightsDescriptorHolder.init(1, vk::ShaderStageFlagBits::eFragment);
    m_lightsDescriptorSet = m_lightsDescriptorHolder.allocateSet();
}

void DeepDeferredStage::initClearPass()
{
    //----- Shaders

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/fullscreen.vert");
    m_clearPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["GBUFFER_MAX_NODE_DEPTH"] = GBUFFER_MAX_NODE_DEPTH_STRING;
    auto fragmentShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/deep-deferred-clear.frag", moduleOptions);
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
    m_geometryPipelineHolder.add(m_lightsDescriptorHolder.setLayout());
    m_geometryPipelineHolder.add(m_scene.cameraDescriptorHolder().setLayout());
    m_geometryPipelineHolder.add(m_scene.materialDescriptorHolder().setLayout());
    m_geometryPipelineHolder.add(m_scene.meshDescriptorHolder().setLayout());

    //----- Rasterization

    m_geometryPipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = findDepthBufferFormat(m_scene.engine().physicalDevice());
    m_geometryPipelineHolder.set(depthStencilAttachment);

    vulkan::PipelineHolder::ColorAttachment gBufferNodeColorAttachment;
    gBufferNodeColorAttachment.format = vk::Format::eR32G32B32A32Uint;
    gBufferNodeColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // There are multiple nodes render targets
    m_geometryPipelineHolder.add(gBufferNodeColorAttachment);
    m_geometryPipelineHolder.add(gBufferNodeColorAttachment);
    m_geometryPipelineHolder.add(gBufferNodeColorAttachment);

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
    m_epiphanyPipelineHolder.add(m_lightsDescriptorHolder.setLayout());
    m_epiphanyPipelineHolder.add(m_scene.cameraDescriptorHolder().setLayout());

    //----- Uniform buffers

    m_lightsUboHolder.init(m_lightsDescriptorSet, m_lightsDescriptorHolder.uniformBufferBindingOffset(),
                             {sizeof(EpiphanyPointLightUbo)});

    //----- Attachments

    vulkan::PipelineHolder::InputAttachment gBufferInputNodeAttachment;
    gBufferInputNodeAttachment.format = vk::Format::eR32G32B32A32Uint;

    // There are multiple render targets
    m_epiphanyPipelineHolder.add(gBufferInputNodeAttachment);
    m_epiphanyPipelineHolder.add(gBufferInputNodeAttachment);
    m_epiphanyPipelineHolder.add(gBufferInputNodeAttachment);

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    m_epiphanyPipelineHolder.add(finalColorAttachment);
}

void DeepDeferredStage::updateGeometryPassShaders(bool firstTime)
{
    m_geometryPipelineHolder.removeShaderStages();

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/deep-deferred-geometry.vert");
    m_geometryPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["GBUFFER_MAX_NODE_DEPTH"] = GBUFFER_MAX_NODE_DEPTH_STRING;
    if (firstTime) moduleOptions.updateCallback = [this]() { updateGeometryPassShaders(false); };
    auto fragmentShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/deep-deferred-geometry.frag", moduleOptions);
    m_geometryPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    if (!firstTime) {
        m_geometryPipelineHolder.update(m_extent);
    }
}

void DeepDeferredStage::updateEpiphanyPassShaders(bool firstTime)
{
    m_epiphanyPipelineHolder.removeShaderStages();

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/fullscreen.vert");
    m_epiphanyPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["GBUFFER_MAX_NODE_DEPTH"] = GBUFFER_MAX_NODE_DEPTH_STRING;
    if (firstTime) moduleOptions.updateCallback = [this]() { updateEpiphanyPassShaders(false); };
    auto fragmentShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/deep-deferred-epiphany.frag", moduleOptions);
    m_epiphanyPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    if (!firstTime) {
        m_epiphanyPipelineHolder.update(m_extent);
    }
}

void DeepDeferredStage::createResources()
{
    // GBuffer Input
    m_gBufferInputNodeImageHolders.clear();
    m_gBufferInputNodeImageHolders.reserve(3u);
    auto gBufferHeaderFormat = vk::Format::eR32G32B32A32Uint;
    for (auto i = 0u; i < 3u; ++i) {
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
        1u * sizeof(uint32_t) + GBUFFER_MAX_NODE_DEPTH * m_extent.width * m_extent.height * sizeof(GBufferNode);
    m_gBufferSsboListBufferHolder.create(vk::BufferUsageFlagBits::eStorageBuffer, listSize);
    m_gBufferSsboDescriptorHolder.updateSet(m_gBufferSsboDescriptorSet, m_gBufferSsboListBufferHolder.buffer(), listSize, 1);

    logger.info("magma.vulkan.stages.deep-deferred-stage")
        << "GBuffer sizes | header: " << headerSize / 1000000.f << "Mo | list: " << listSize / 1000000.f << "Mo." << std::endl;

    // Final
    auto finalFormat = vk::Format::eR8G8B8A8Unorm;
    m_finalImageHolder.create(finalFormat, m_extent, vk::ImageAspectFlagBits::eColor);

    // Depth
    auto depthFormat = findDepthBufferFormat(m_scene.engine().physicalDevice());
    m_depthImageHolder.create(depthFormat, m_extent, vk::ImageAspectFlagBits::eDepth);
}

void DeepDeferredStage::createFramebuffers()
{
    // Framebuffer
    std::array<vk::ImageView, 8> attachments = {m_gBufferInputNodeImageHolders[0].view(),
                                                m_gBufferInputNodeImageHolders[1].view(),
                                                m_gBufferInputNodeImageHolders[2].view(),
                                                m_depthImageHolder.view(),
                                                m_finalImageHolder.view(),
                                                m_gBufferInputNodeImageHolders[0].view(),
                                                m_gBufferInputNodeImageHolders[1].view(),
                                                m_gBufferInputNodeImageHolders[2].view()};

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

void DeepDeferredStage::updateEpiphanyUbo()
{
    if (m_scene.lights().size() > 0) {
        const auto& light = m_scene.light(0);

        if (light.type() == LightType::Point) {
            const auto& pointLight = reinterpret_cast<const PointLight::Impl&>(light);

            EpiphanyPointLightUbo ubo;
            ubo.wPosition = glm::vec4(pointLight.position(), 1.f);
            ubo.radius = pointLight.radius();

            m_lightsUboHolder.copy(0, ubo);
        }
    }
}
