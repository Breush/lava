#include "./forward-renderer-stage.hpp"

#include <lava/magma/camera.hpp>
#include <lava/magma/light.hpp>
#include <lava/magma/mesh.hpp>
#include <lava/magma/scene.hpp>
#include <lava/magma/vertex.hpp>

#include "../../aft-vulkan/camera-aft.hpp"
#include "../../aft-vulkan/light-aft.hpp"
#include "../../aft-vulkan/mesh-aft.hpp"
#include "../../aft-vulkan/scene-aft.hpp"
#include "../../g-buffer-data.hpp"
#include "../../helpers/frustum.hpp"
#include "../environment.hpp"
#include "../helpers/format.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

ForwardRendererStage::ForwardRendererStage(Scene& scene)
    : m_scene(scene)
    , m_renderPassHolder(m_scene.engine().impl())
    , m_opaquePipelineHolder(m_scene.engine().impl())
    , m_depthlessPipelineHolder(m_scene.engine().impl())
    , m_wireframePipelineHolder(m_scene.engine().impl())
    , m_translucentPipelineHolder(m_scene.engine().impl())
    , m_finalImageHolder(m_scene.engine().impl(), "magma.vulkan.stages.forward-renderer.final-image")
    , m_depthImageHolder(m_scene.engine().impl(), "magma.vulkan.stages.forward-renderer.depth-image")
    , m_framebuffer(m_scene.engine().impl().device())
{
}

void ForwardRendererStage::init(const Camera& camera)
{
    m_camera = &camera;

    logger.info("magma.vulkan.stages.forward-renderer-stage") << "Initializing." << std::endl;
    logger.log().tab(1);

    updatePassShaders(true);
    initOpaquePass();
    initDepthlessPass();
    initWireframePass();
    initTranslucentPass();

    //----- Render pass

    m_renderPassHolder.add(m_opaquePipelineHolder);
    m_renderPassHolder.add(m_depthlessPipelineHolder);
    m_renderPassHolder.add(m_wireframePipelineHolder);
    m_renderPassHolder.add(m_translucentPipelineHolder);
    m_renderPassHolder.init();

    logger.log().tab(-1);
}

void ForwardRendererStage::update(vk::Extent2D extent, vk::PolygonMode polygonMode)
{
    m_extent = extent;
    m_polygonMode = polygonMode;

    m_opaquePipelineHolder.update(extent, polygonMode);
    m_depthlessPipelineHolder.update(extent, polygonMode);
    m_wireframePipelineHolder.update(extent, vk::PolygonMode::eLine);
    m_translucentPipelineHolder.update(extent, polygonMode);

    createResources();
    createFramebuffers();
}

void ForwardRendererStage::render(vk::CommandBuffer commandBuffer, uint32_t frameId)
{
    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    const auto& deviceHolder = m_scene.engine().impl().deviceHolder();
    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer");

    //----- Prologue

    // @todo These clearValues could be returned by a method of RenderPassHolder,
    // making sure it has the right size and with default values already set
    // during construction.

    // Set render pass
    std::array<vk::ClearValue, 2> clearValues;
    // @todo Allow clear color to be configurable
    std::array<float, 4> clearColor{0.2f, 0.6f, 0.4f, 1.f};
    clearValues[0u].color = vk::ClearColorValue(clearColor);
    clearValues[1u].depthStencil = vk::ClearDepthStencilValue{1.f, 0u};

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPassHolder.renderPass();
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    //----- Opaque pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.opaque");

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_opaquePipelineHolder.pipeline());

    // Bind lights
    for (auto light : m_scene.lights()) {
        light->aft().render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), LIGHTS_DESCRIPTOR_SET_INDEX);
        m_scene.aft()
            .shadows(*light, *m_camera)
            .render(commandBuffer, frameId, m_opaquePipelineHolder.pipelineLayout(), SHADOWS_DESCRIPTOR_SET_INDEX);
    }

    // Set the camera
    m_camera->aft().render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), CAMERA_PUSH_CONSTANT_OFFSET);
    const auto& cameraFrustum = m_camera->frustum();

    // Set the environment
    m_scene.aft().environment().render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), ENVIRONMENT_DESCRIPTOR_SET_INDEX);

    // Draw all opaque meshes and sort others
    std::vector<const Mesh*> depthlessMeshes;
    std::vector<const Mesh*> wireframedMeshes;
    std::vector<const Mesh*> translucentMeshes;
    for (auto mesh : m_scene.meshes()) {
        if (m_camera->vrAimed() && !mesh->vrRenderable()) continue;
        if (mesh->depthless()) {
            depthlessMeshes.emplace_back(mesh);
            continue;
        }
        if (mesh->wireframed()) {
            wireframedMeshes.emplace_back(mesh);
            continue;
        }
        if (mesh->translucent()) {
            translucentMeshes.emplace_back(mesh);
            continue;
        }

        const auto& boundingSphere = mesh->boundingSphere();
        if (!m_camera->frustumCullingEnabled() || helpers::isVisibleInsideFrustum(boundingSphere, cameraFrustum)) {
            tracker.counter("draw-calls.renderer") += 1u;
            mesh->aft().render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), MESH_PUSH_CONSTANT_OFFSET,
                               MATERIAL_DESCRIPTOR_SET_INDEX);
        }
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Depthless pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.depthless");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_depthlessPipelineHolder.pipeline());

    for (auto mesh : depthlessMeshes) {
        tracker.counter("draw-calls.renderer") += 1u;
        mesh->aft().render(commandBuffer, m_depthlessPipelineHolder.pipelineLayout(), MESH_PUSH_CONSTANT_OFFSET,
                           MATERIAL_DESCRIPTOR_SET_INDEX);
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Wireframe pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.wireframe");

    // @todo No need to bind wireframe if there is nothing to draw in it.
    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_wireframePipelineHolder.pipeline());

    // Draw all wireframed meshes
    for (auto mesh : wireframedMeshes) {
        const auto& boundingSphere = mesh->boundingSphere();
        if (!m_camera->frustumCullingEnabled() || helpers::isVisibleInsideFrustum(boundingSphere, cameraFrustum)) {
            tracker.counter("draw-calls.renderer") += 1u;
            mesh->aft().renderUnlit(commandBuffer, m_wireframePipelineHolder.pipelineLayout(), MESH_PUSH_CONSTANT_OFFSET);
        }
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Translucent pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.translucent");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_translucentPipelineHolder.pipeline());

    // Draw all translucent meshes
    // @fixme We should sort the meshes
    // https://github.com/Breush/lava/issues/36
    for (auto mesh : translucentMeshes) {
        const auto& boundingSphere = mesh->boundingSphere();
        if (!m_camera->frustumCullingEnabled() || helpers::isVisibleInsideFrustum(boundingSphere, cameraFrustum)) {
            tracker.counter("draw-calls.renderer") += 1u;
            mesh->aft().render(commandBuffer, m_translucentPipelineHolder.pipelineLayout(), MESH_PUSH_CONSTANT_OFFSET,
                               MATERIAL_DESCRIPTOR_SET_INDEX);
        }
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Epilogue

    commandBuffer.endRenderPass();

    deviceHolder.debugEndRegion(commandBuffer);
}

RenderImage ForwardRendererStage::renderImage() const
{
    auto cameraId = m_scene.aft().cameraId(*m_camera);
    return m_finalImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA + cameraId);
}

RenderImage ForwardRendererStage::depthRenderImage() const
{
    auto cameraId = m_scene.aft().cameraId(*m_camera);
    return m_depthImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA_DEPTH + cameraId);
}

void ForwardRendererStage::changeRenderImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer)
{
    m_finalImageHolder.changeLayout(imageLayout, commandBuffer);
}

//----- Internal

void ForwardRendererStage::initOpaquePass()
{
    //----- Descriptor set layouts

    // @note Ordering is important
    m_opaquePipelineHolder.add(m_scene.aft().environmentDescriptorHolder().setLayout());
    m_opaquePipelineHolder.add(m_scene.aft().materialDescriptorHolder().setLayout());
    m_opaquePipelineHolder.add(m_scene.aft().lightsDescriptorHolder().setLayout());
    m_opaquePipelineHolder.add(m_scene.aft().shadowsDescriptorHolder().setLayout());

    //----- Push constants

    m_opaquePipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_opaquePipelineHolder.addPushConstantRange(sizeof(CameraUbo));

    //----- Rasterization

    m_opaquePipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
    m_opaquePipelineHolder.set(depthStencilAttachment);

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    m_opaquePipelineHolder.add(finalColorAttachment);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tangent)}};
    m_opaquePipelineHolder.set(vertexInput);
}

void ForwardRendererStage::initDepthlessPass()
{
    //----- Descriptor set layouts

    // @note Ordering is important
    m_depthlessPipelineHolder.add(m_scene.aft().environmentDescriptorHolder().setLayout());
    m_depthlessPipelineHolder.add(m_scene.aft().materialDescriptorHolder().setLayout());
    m_depthlessPipelineHolder.add(m_scene.aft().lightsDescriptorHolder().setLayout());
    m_depthlessPipelineHolder.add(m_scene.aft().shadowsDescriptorHolder().setLayout());

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

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    finalColorAttachment.blending = vulkan::PipelineHolder::ColorAttachmentBlending::AlphaBlending;
    m_depthlessPipelineHolder.add(finalColorAttachment);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tangent)}};
    m_depthlessPipelineHolder.set(vertexInput);
}

void ForwardRendererStage::initWireframePass()
{
    //----- Descriptor set layouts

    // @note Ordering is important
    m_wireframePipelineHolder.add(m_scene.aft().environmentDescriptorHolder().setLayout());
    m_wireframePipelineHolder.add(m_scene.aft().materialDescriptorHolder().setLayout());
    // @note Useful, so that the pipelines are all compatible
    m_wireframePipelineHolder.add(m_scene.aft().lightsDescriptorHolder().setLayout());
    m_wireframePipelineHolder.add(m_scene.aft().shadowsDescriptorHolder().setLayout());

    //----- Push constants

    m_wireframePipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_wireframePipelineHolder.addPushConstantRange(sizeof(CameraUbo));

    //----- Rasterization

    m_wireframePipelineHolder.set(vk::CullModeFlagBits::eNone);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
    depthStencilAttachment.depthWriteEnabled = false;
    depthStencilAttachment.clear = false;
    m_wireframePipelineHolder.set(depthStencilAttachment);

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    finalColorAttachment.blending = vulkan::PipelineHolder::ColorAttachmentBlending::AlphaBlending;
    m_wireframePipelineHolder.add(finalColorAttachment);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(UnlitVertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(UnlitVertex, pos)}};
    m_wireframePipelineHolder.set(vertexInput);
}

void ForwardRendererStage::initTranslucentPass()
{
    // @note Translucent pass differs from opaque pass just by having
    // depth writing disabled and alpha blending enabled.

    //----- Descriptor set layouts

    // @note Ordering is important
    m_translucentPipelineHolder.add(m_scene.aft().environmentDescriptorHolder().setLayout());
    m_translucentPipelineHolder.add(m_scene.aft().materialDescriptorHolder().setLayout());
    m_translucentPipelineHolder.add(m_scene.aft().lightsDescriptorHolder().setLayout());
    m_translucentPipelineHolder.add(m_scene.aft().shadowsDescriptorHolder().setLayout());

    //----- Push constants

    m_translucentPipelineHolder.addPushConstantRange(sizeof(MeshUbo));
    m_translucentPipelineHolder.addPushConstantRange(sizeof(CameraUbo));

    //----- Rasterization

    m_translucentPipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
    depthStencilAttachment.depthWriteEnabled = false;
    depthStencilAttachment.clear = false;
    m_translucentPipelineHolder.set(depthStencilAttachment);

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    finalColorAttachment.blending = vulkan::PipelineHolder::ColorAttachmentBlending::AlphaBlending;
    m_translucentPipelineHolder.add(finalColorAttachment);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tangent)}};
    m_translucentPipelineHolder.set(vertexInput);
}

void ForwardRendererStage::updatePassShaders(bool firstTime)
{
    // @note We use the very same shaders for opaque and translucent meshes,
    // only the depth test and the color attachments differ.

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["USE_CAMERA_PUSH_CONSTANT"] = "1";
    moduleOptions.defines["USE_MESH_PUSH_CONSTANT"] = "1";
    moduleOptions.defines["USE_SHADOW_MAP_PUSH_CONSTANT"] = "0";
    moduleOptions.defines["MATERIAL_DESCRIPTOR_SET_INDEX"] = std::to_string(MATERIAL_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["LIGHTS_DESCRIPTOR_SET_INDEX"] = std::to_string(LIGHTS_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["ENVIRONMENT_DESCRIPTOR_SET_INDEX"] = std::to_string(ENVIRONMENT_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT"] = std::to_string(ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT);
    moduleOptions.defines["SHADOWS_DESCRIPTOR_SET_INDEX"] = std::to_string(SHADOWS_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["SHADOWS_CASCADES_COUNT"] = std::to_string(SHADOWS_CASCADES_COUNT);
    moduleOptions.defines["LIGHT_TYPE_POINT"] = std::to_string(static_cast<uint32_t>(LightType::Point));
    moduleOptions.defines["LIGHT_TYPE_DIRECTIONAL"] = std::to_string(static_cast<uint32_t>(LightType::Directional));
    moduleOptions.defines["MATERIAL_DATA_SIZE"] = std::to_string(MATERIAL_DATA_SIZE);
    moduleOptions.defines["MATERIAL_SAMPLERS_SIZE"] = std::to_string(MATERIAL_SAMPLERS_SIZE);
    moduleOptions.defines["G_BUFFER_DATA_SIZE"] = std::to_string(G_BUFFER_DATA_SIZE);
    if (firstTime) moduleOptions.updateCallback = [this]() { updatePassShaders(false); };

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/geometry.vert", moduleOptions);
    auto fragmentShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/renderers/forward/geometry.frag", moduleOptions);

    auto depthlessVertexShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/geometry-depthless.vert", moduleOptions);

    auto unlitVertexShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/geometry-unlit.vert", moduleOptions);
    auto unlitFragmentShaderModule =
        m_scene.engine().impl().shadersManager().module("./data/shaders/stages/geometry-unlit.frag", moduleOptions);

    m_opaquePipelineHolder.removeShaderStages();
    m_opaquePipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});
    m_opaquePipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    m_depthlessPipelineHolder.removeShaderStages();
    m_depthlessPipelineHolder.add(
        {shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, depthlessVertexShaderModule, "main"});
    m_depthlessPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    m_wireframePipelineHolder.removeShaderStages();
    m_wireframePipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, unlitVertexShaderModule, "main"});
    m_wireframePipelineHolder.add(
        {shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, unlitFragmentShaderModule, "main"});

    m_translucentPipelineHolder.removeShaderStages();
    m_translucentPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});
    m_translucentPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    if (!firstTime) {
        m_opaquePipelineHolder.update(m_extent, m_polygonMode);
        m_wireframePipelineHolder.update(m_extent, vk::PolygonMode::eLine);
        m_translucentPipelineHolder.update(m_extent, m_polygonMode);
    }
}

void ForwardRendererStage::createResources()
{
    // Final
    auto finalFormat = vk::Format::eR8G8B8A8Unorm;
    m_finalImageHolder.create(finalFormat, m_extent, vk::ImageAspectFlagBits::eColor);

    // Depth
    auto depthFormat = vulkan::depthBufferFormat(m_scene.engine().impl().physicalDevice());
    m_depthImageHolder.create(depthFormat, m_extent, vk::ImageAspectFlagBits::eDepth);
}

void ForwardRendererStage::createFramebuffers()
{
    // Attachments
    std::vector<vk::ImageView> attachments;
    attachments.emplace_back(m_finalImageHolder.view());
    attachments.emplace_back(m_depthImageHolder.view());
    attachments.emplace_back(m_finalImageHolder.view());
    attachments.emplace_back(m_depthImageHolder.view());
    attachments.emplace_back(m_finalImageHolder.view());
    attachments.emplace_back(m_depthImageHolder.view());
    attachments.emplace_back(m_finalImageHolder.view());
    attachments.emplace_back(m_depthImageHolder.view());

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
        logger.error("magma.vulkan.stages.forward-renderer-stage") << "Failed to create framebuffers." << std::endl;
    }
}
