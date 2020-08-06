#include "./deep-deferred-stage.hpp"

#include <lava/magma/camera.hpp>
#include <lava/magma/light.hpp>
#include <lava/magma/mesh.hpp>
#include <lava/magma/scene.hpp>
#include <lava/magma/vertex.hpp>

#include "../../aft-vulkan/camera-aft.hpp"
#include "../../aft-vulkan/light-aft.hpp"
#include "../../aft-vulkan/mesh-aft.hpp"
#include "../../aft-vulkan/scene-aft.hpp"
#include "../helpers/format.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

DeepDeferredStage::DeepDeferredStage(Scene& scene)
    : m_scene(scene)
    , m_renderPassHolder(m_scene.engine().impl())
    , m_clearPipelineHolder(m_scene.engine().impl())
    , m_geometryPipelineHolder(m_scene.engine().impl())
    , m_depthlessPipelineHolder(m_scene.engine().impl())
    , m_epiphanyPipelineHolder(m_scene.engine().impl())
    , m_gBufferInputDescriptorHolder(m_scene.engine().impl())
    , m_gBufferSsboDescriptorHolder(m_scene.engine().impl())
    , m_gBufferSsboHeaderBufferHolder(m_scene.engine().impl())
    , m_gBufferSsboListBufferHolder(m_scene.engine().impl())
    , m_finalImageHolder(m_scene.engine().impl(), "magma.vulkan.stages.deep-deferred.final-image")
    , m_depthImageHolder(m_scene.engine().impl(), "magma.vulkan.stages.deep-deferred.depth-image")
    , m_framebuffer(m_scene.engine().impl().device())
{
}

void DeepDeferredStage::init(const Camera& camera)
{
    m_camera = &camera;

    logger.info("magma.vulkan.stages.deep-deferred") << "Initializing." << std::endl;
    logger.log().tab(1);

    initGBuffer();
    initClearPass();
    initGeometryPass();
    initDepthlessPass();
    initEpiphanyPass();

    //----- Render pass

    m_renderPassHolder.add(m_clearPipelineHolder);
    m_renderPassHolder.add(m_geometryPipelineHolder);
    m_renderPassHolder.add(m_depthlessPipelineHolder);
    m_renderPassHolder.add(m_epiphanyPipelineHolder);

    logger.log().tab(-1);
}

void DeepDeferredStage::rebuild()
{
    if (m_rebuildRenderPass) {
        m_renderPassHolder.init();
        m_rebuildRenderPass = false;
    }

    if (m_rebuildPipelines) {
        m_clearPipelineHolder.update(m_extent);
        m_geometryPipelineHolder.update(m_extent, m_polygonMode);
        m_depthlessPipelineHolder.update(m_extent, m_polygonMode);
        m_epiphanyPipelineHolder.update(m_extent);
        m_rebuildPipelines = false;
    }

    if (m_rebuildResources) {
        createResources();
        createFramebuffers();
        m_rebuildResources = false;
    }
}

void DeepDeferredStage::record(vk::CommandBuffer commandBuffer, uint32_t frameId)
{
    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    const auto& deviceHolder = m_scene.engine().impl().deviceHolder();
    deviceHolder.debugBeginRegion(commandBuffer, "deep-deferred");

    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 3u * DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT + 2u> clearValues;

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
    commandBuffer.draw(3, 1, 0, 0);

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Geometry pass

    deviceHolder.debugBeginRegion(commandBuffer, "deep-deferred.geometry");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_geometryPipelineHolder.pipeline());

    // Set the camera
    m_camera->aft().render(commandBuffer, m_geometryPipelineHolder.pipelineLayout(), CAMERA_PUSH_CONSTANT_OFFSET);
    const auto& cameraFrustum = m_camera->frustum();

    // Draw all meshes
    std::vector<const Mesh*> depthlessMeshes;
    for (auto mesh : m_scene.meshes()) {
        if (m_camera->vrAimed() && !mesh->vrRenderable()) continue;

        // @todo Somehow, the deep-deferred renderer does not care about wireframes.
        if (mesh->category() == RenderCategory::Depthless) {
            depthlessMeshes.emplace_back(mesh);
            continue;
        }

        const auto& boundingSphere = mesh->boundingSphere();
        if (!m_camera->frustumCullingEnabled() || cameraFrustum.canSee(boundingSphere)) {
            tracker.counter("draw-calls.renderer") += 1u;
            mesh->aft().render(commandBuffer, m_geometryPipelineHolder.pipelineLayout(), GEOMETRY_MESH_PUSH_CONSTANT_OFFSET,
                               GEOMETRY_MATERIAL_DESCRIPTOR_SET_INDEX);
        }
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Depthless pass

    deviceHolder.debugBeginRegion(commandBuffer, "deep-deferred.depthless");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_depthlessPipelineHolder.pipeline());

    // Draw all meshes
    for (auto mesh : depthlessMeshes) {
        if (m_camera->vrAimed() && !mesh->vrRenderable()) continue;
        const auto& boundingSphere = mesh->boundingSphere();
        if (!m_camera->frustumCullingEnabled() || cameraFrustum.canSee(boundingSphere)) {
            tracker.counter("draw-calls.renderer") += 1u;
            mesh->aft().render(commandBuffer, m_depthlessPipelineHolder.pipelineLayout(), GEOMETRY_MESH_PUSH_CONSTANT_OFFSET,
                               GEOMETRY_MATERIAL_DESCRIPTOR_SET_INDEX);
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

    m_scene.aft().environment().render(commandBuffer, m_epiphanyPipelineHolder.pipelineLayout(),
                                       EPIPHANY_ENVIRONMENT_DESCRIPTOR_SET_INDEX);

    // Bind lights and shadows
    for (auto light : m_scene.lights()) {
        light->aft().render(commandBuffer, m_epiphanyPipelineHolder.pipelineLayout(), EPIPHANY_LIGHTS_DESCRIPTOR_SET_INDEX);
        m_scene.aft()
            .shadows(*light, *m_camera)
            .render(commandBuffer, frameId, m_epiphanyPipelineHolder.pipelineLayout(), EPIPHANY_SHADOWS_DESCRIPTOR_SET_INDEX);
    }

    commandBuffer.draw(3, 1, 0, 1);

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Epilogue

    commandBuffer.endRenderPass();

    deviceHolder.debugEndRegion(commandBuffer);
}

void DeepDeferredStage::extent(const vk::Extent2D& extent)
{
    if (m_extent == extent) return;
    m_extent = extent;
    m_rebuildPipelines = true;
    m_rebuildResources = true;
}

void DeepDeferredStage::polygonMode(vk::PolygonMode polygonMode)
{
    if (m_polygonMode == polygonMode) return;
    m_polygonMode = polygonMode;
    m_rebuildPipelines = true;
}

RenderImage DeepDeferredStage::renderImage() const
{
    auto cameraId = m_scene.aft().cameraId(*m_camera);
    auto renderImage = m_finalImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA + cameraId);
    renderImage.impl().channelCount(3u); // Alpha is never written with this render pass
    return renderImage;
}

RenderImage DeepDeferredStage::depthRenderImage() const
{
    auto cameraId = m_scene.aft().cameraId(*m_camera);
    return m_depthImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA_DEPTH + cameraId);
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
    auto vertexShaderModule = m_scene.engine().impl().shadersManager().module("./data/shaders/stages/fullscreen.vert");
    m_clearPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["USE_CAMERA_PUSH_CONSTANT"] = '1';
    moduleOptions.defines["USE_FLAT_PUSH_CONSTANT"] = '0';
    moduleOptions.defines["USE_MESH_PUSH_CONSTANT"] = '1';
    moduleOptions.defines["USE_SHADOW_MAP_PUSH_CONSTANT"] = '0';
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH"] = std::to_string(DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["MESH_PUSH_CONSTANT_OFFSET"] = std::to_string(GEOMETRY_MESH_PUSH_CONSTANT_OFFSET);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT);
    moduleOptions.defines["G_BUFFER_DATA_SIZE"] = std::to_string(G_BUFFER_DATA_SIZE);
    auto fragmentShaderModule = m_scene.engine().impl().shadersManager().module(
        "./data/shaders/stages/renderers/deep-deferred/clear.frag", moduleOptions);
    m_clearPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    //----- Descriptor set layouts

    m_clearPipelineHolder.add(m_gBufferInputDescriptorHolder.setLayout());
    m_clearPipelineHolder.add(m_gBufferSsboDescriptorHolder.setLayout());

    //----- Push constants

    m_clearPipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_clearPipelineHolder.addPushConstantRange(sizeof(CameraUbo));
}

void DeepDeferredStage::initGeometryPass()
{
    //----- Shaders

    updateGeometryPassShaders(true);

    //----- Descriptor set layouts

    // @note Ordering is important
    m_geometryPipelineHolder.add(m_gBufferInputDescriptorHolder.setLayout());
    m_geometryPipelineHolder.add(m_gBufferSsboDescriptorHolder.setLayout());
    m_geometryPipelineHolder.add(m_scene.aft().materialDescriptorHolder().setLayout());

    //----- Push constants

    m_geometryPipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_geometryPipelineHolder.addPushConstantRange(sizeof(CameraUbo));

    //----- Rasterization

    m_geometryPipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
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
    vertexInput.stride = sizeof(Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tangent)}};
    m_geometryPipelineHolder.set(vertexInput);
}

void DeepDeferredStage::initDepthlessPass()
{
    //----- Descriptor set layouts

    // @note Ordering is important
    m_depthlessPipelineHolder.add(m_gBufferInputDescriptorHolder.setLayout());
    m_depthlessPipelineHolder.add(m_gBufferSsboDescriptorHolder.setLayout());
    m_depthlessPipelineHolder.add(m_scene.aft().materialDescriptorHolder().setLayout());

    //----- Push constants

    m_depthlessPipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_depthlessPipelineHolder.addPushConstantRange(sizeof(CameraUbo));

    //----- Rasterization

    m_depthlessPipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
    depthStencilAttachment.depthWriteEnabled = false;
    depthStencilAttachment.clear = false;
    m_depthlessPipelineHolder.set(depthStencilAttachment);

    vulkan::PipelineHolder::ColorAttachment gBufferNodeColorAttachment;
    gBufferNodeColorAttachment.format = vk::Format::eR32G32B32A32Uint;
    gBufferNodeColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    gBufferNodeColorAttachment.clear = false;

    // There are multiple nodes render targets
    for (auto i = 0u; i < DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT; ++i) {
        m_depthlessPipelineHolder.add(gBufferNodeColorAttachment);
    }

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tangent)}};
    m_depthlessPipelineHolder.set(vertexInput);
}

void DeepDeferredStage::initEpiphanyPass()
{
    //----- Shaders

    updateEpiphanyPassShaders(true);

    //----- Descriptors

    m_epiphanyPipelineHolder.add(m_gBufferInputDescriptorHolder.setLayout());
    m_epiphanyPipelineHolder.add(m_gBufferSsboDescriptorHolder.setLayout());
    m_epiphanyPipelineHolder.add(m_scene.aft().environmentDescriptorHolder().setLayout());
    m_epiphanyPipelineHolder.add(m_scene.aft().lightsDescriptorHolder().setLayout());
    m_epiphanyPipelineHolder.add(m_scene.aft().shadowsDescriptorHolder().setLayout());

    //----- Push constants

    m_epiphanyPipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_epiphanyPipelineHolder.addPushConstantRange(sizeof(CameraUbo));

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
    m_depthlessPipelineHolder.removeShaderStages();

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["USE_CAMERA_PUSH_CONSTANT"] = '1';
    moduleOptions.defines["USE_FLAT_PUSH_CONSTANT"] = '0';
    moduleOptions.defines["USE_MESH_PUSH_CONSTANT"] = '1';
    moduleOptions.defines["USE_SHADOW_MAP_PUSH_CONSTANT"] = '0';
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH"] = std::to_string(DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT);
    moduleOptions.defines["MATERIAL_DESCRIPTOR_SET_INDEX"] = std::to_string(GEOMETRY_MATERIAL_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["MATERIAL_GLOBAL_DESCRIPTOR_SET_INDEX"] = std::to_string(GEOMETRY_MATERIAL_GLOBAL_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["MESH_PUSH_CONSTANT_OFFSET"] = std::to_string(GEOMETRY_MESH_PUSH_CONSTANT_OFFSET);
    moduleOptions.defines["MATERIAL_DATA_SIZE"] = std::to_string(MATERIAL_DATA_SIZE);
    moduleOptions.defines["MATERIAL_SAMPLERS_SIZE"] = std::to_string(MATERIAL_SAMPLERS_SIZE);
    moduleOptions.defines["MATERIAL_GLOBAL_SAMPLERS_SIZE"] = std::to_string(MATERIAL_SAMPLERS_SIZE);
    moduleOptions.defines["G_BUFFER_DATA_SIZE"] = std::to_string(G_BUFFER_DATA_SIZE);
    if (firstTime) moduleOptions.updateCallback = [this]() { updateGeometryPassShaders(false); };

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/geometry.vert", moduleOptions);
    auto fragmentShaderModule = m_scene.engine().impl().shadersManager().module(
        "./data/shaders/stages/renderers/deep-deferred/geometry.frag", moduleOptions);

    auto depthlessVertexShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/geometry-depthless.vert", moduleOptions);

    m_geometryPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});
    m_geometryPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    m_depthlessPipelineHolder.add(
        {shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, depthlessVertexShaderModule, "main"});
    m_depthlessPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    if (!firstTime) {
        m_geometryPipelineHolder.update(m_extent);
        m_depthlessPipelineHolder.update(m_extent);
    }
}

void DeepDeferredStage::updateEpiphanyPassShaders(bool firstTime)
{
    m_epiphanyPipelineHolder.removeShaderStages();

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["USE_CAMERA_PUSH_CONSTANT"] = '1';
    moduleOptions.defines["USE_FLAT_PUSH_CONSTANT"] = '0';
    moduleOptions.defines["USE_MESH_PUSH_CONSTANT"] = '1';
    moduleOptions.defines["USE_SHADOW_MAP_PUSH_CONSTANT"] = '0';
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH"] = std::to_string(DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_INPUT_DESCRIPTOR_SET_INDEX"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_INPUT_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT"] =
        std::to_string(DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT);
    moduleOptions.defines["ENVIRONMENT_DESCRIPTOR_SET_INDEX"] = std::to_string(EPIPHANY_ENVIRONMENT_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT"] = std::to_string(ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT);
    moduleOptions.defines["MESH_PUSH_CONSTANT_OFFSET"] = std::to_string(GEOMETRY_MESH_PUSH_CONSTANT_OFFSET);
    moduleOptions.defines["LIGHTS_DESCRIPTOR_SET_INDEX"] = std::to_string(EPIPHANY_LIGHTS_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["SHADOWS_DESCRIPTOR_SET_INDEX"] = std::to_string(EPIPHANY_SHADOWS_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["SHADOWS_CASCADES_COUNT"] = std::to_string(SHADOWS_CASCADES_COUNT);
    moduleOptions.defines["LIGHT_TYPE_POINT"] = std::to_string(static_cast<uint32_t>(LightType::Point));
    moduleOptions.defines["LIGHT_TYPE_DIRECTIONAL"] = std::to_string(static_cast<uint32_t>(LightType::Directional));
    moduleOptions.defines["MATERIAL_DATA_SIZE"] = std::to_string(MATERIAL_DATA_SIZE);
    moduleOptions.defines["MATERIAL_SAMPLERS_SIZE"] = std::to_string(MATERIAL_SAMPLERS_SIZE);
    moduleOptions.defines["G_BUFFER_DATA_SIZE"] = std::to_string(G_BUFFER_DATA_SIZE);
    if (firstTime) moduleOptions.updateCallback = [this]() { updateEpiphanyPassShaders(false); };

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().impl().shadersManager().module("./data/shaders/stages/fullscreen.vert");
    m_epiphanyPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    auto fragmentShaderModule = m_scene.engine().impl().shadersManager().module(
        "./data/shaders/stages/renderers/deep-deferred/epiphany.frag", moduleOptions);
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
        m_gBufferInputNodeImageHolders.emplace_back(std::make_unique<vulkan::ImageHolder>(m_scene.engine().impl()));
        m_gBufferInputNodeImageHolders[i]->create(gBufferHeaderFormat, m_extent, vk::ImageAspectFlagBits::eColor);
        m_gBufferInputDescriptorHolder.updateSet(m_gBufferInputDescriptorSet, m_gBufferInputNodeImageHolders[i]->view(),
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

    logger.info("magma.vulkan.stages.deep-deferred")
        << "GBuffer sizes | header: " << headerSize / 1000000.f << "Mo | list: " << listSize / 1000000.f << "Mo." << std::endl;

    // Final
    auto finalFormat = vk::Format::eR8G8B8A8Unorm;
    m_finalImageHolder.create(finalFormat, m_extent, vk::ImageAspectFlagBits::eColor);

    // Depth
    auto depthFormat = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
    m_depthImageHolder.create(depthFormat, m_extent, vk::ImageAspectFlagBits::eDepth);
}

void DeepDeferredStage::createFramebuffers()
{
    // Attachments
    std::vector<vk::ImageView> attachments;
    attachments.reserve(3u * (DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT + 1u));

    // Geometry
    for (auto i = 0u; i < DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT; ++i) {
        attachments.emplace_back(m_gBufferInputNodeImageHolders[i]->view());
    }
    attachments.emplace_back(m_depthImageHolder.view());

    // Depthless
    for (auto i = 0u; i < DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT; ++i) {
        attachments.emplace_back(m_gBufferInputNodeImageHolders[i]->view());
    }
    attachments.emplace_back(m_depthImageHolder.view());

    // Epiphany
    attachments.emplace_back(m_finalImageHolder.view());
    for (auto i = 0u; i < DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT; ++i) {
        attachments.emplace_back(m_gBufferInputNodeImageHolders[i]->view());
    }

    // Framebuffer
    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPassHolder.renderPass();
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    if (m_scene.engine().impl().device().createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace())
        != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.deep-deferred") << "Failed to create framebuffers." << std::endl;
    }
}
