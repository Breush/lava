#include "./forward-renderer-stage.hpp"

#include "../../g-buffer-data.hpp"
#include "../../helpers/frustum.hpp"
#include "../cameras/i-camera-impl.hpp"
#include "../environment.hpp"
#include "../helpers/format.hpp"
#include "../lights/i-light-impl.hpp"
#include "../mesh-impl.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../vertex.hpp"

using namespace lava::magma;
using namespace lava::chamber;

ForwardRendererStage::ForwardRendererStage(RenderScene::Impl& scene)
    : m_scene(scene)
    , m_renderPassHolder(m_scene.engine())
    , m_opaquePipelineHolder(m_scene.engine())
    , m_wireframePipelineHolder(m_scene.engine())
    , m_translucentPipelineHolder(m_scene.engine())
    , m_finalImageHolder(m_scene.engine(), "magma.vulkan.stages.forward-renderer.final-image")
    , m_depthImageHolder(m_scene.engine(), "magma.vulkan.stages.forward-renderer.depth-image")
    , m_framebuffer(m_scene.engine().device())
{
}

void ForwardRendererStage::init(uint32_t cameraId)
{
    m_cameraId = cameraId;

    logger.info("magma.vulkan.stages.forward-renderer-stage") << "Initializing." << std::endl;
    logger.log().tab(1);

    updatePassShaders(true);
    initOpaquePass();
    initWireframePass();
    initTranslucentPass();

    //----- Render pass

    m_renderPassHolder.add(m_opaquePipelineHolder);
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
    m_wireframePipelineHolder.update(extent, vk::PolygonMode::eLine);
    m_translucentPipelineHolder.update(extent, polygonMode);

    createResources();
    createFramebuffers();
}

void ForwardRendererStage::render(vk::CommandBuffer commandBuffer)
{
    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    const auto& deviceHolder = m_scene.engine().deviceHolder();
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

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.opaque-pass");

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_opaquePipelineHolder.pipeline());

    // Bind lights
    for (auto lightId = 0u; lightId < m_scene.lightsCount(); ++lightId) {
        m_scene.light(lightId).render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), LIGHTS_DESCRIPTOR_SET_INDEX);
    }

    // Set the camera
    auto& camera = m_scene.camera(m_cameraId);
    camera.render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), CAMERA_PUSH_CONSTANT_OFFSET);
    const auto& cameraFrustum = camera.frustum();

    // Set the environment
    m_scene.environment().render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), ENVIRONMENT_DESCRIPTOR_SET_INDEX);

    // Draw all opaque meshes
    for (auto& mesh : m_scene.meshes()) {
        if (mesh->translucent() || mesh->wireframed() || (camera.vrAimed() && !mesh->vrRenderable())) continue;
        const auto& boundingSphere = mesh->boundingSphere();
        if (!camera.useFrustumCulling() || helpers::isVisibleInsideFrustum(boundingSphere, cameraFrustum)) {
            tracker.counter("draw-calls.renderer") += 1u;
            mesh->render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), MESH_PUSH_CONSTANT_OFFSET,
                         MATERIAL_DESCRIPTOR_SET_INDEX);
        }
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Wireframe pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.wireframe-pass");

    // @todo No need to bind wireframe if there is nothing to draw in it.
    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_wireframePipelineHolder.pipeline());

    // Draw all wireframed meshes
    for (auto& mesh : m_scene.meshes()) {
        if (!mesh->wireframed() || mesh->translucent() || (camera.vrAimed() && !mesh->vrRenderable())) continue;
        const auto& boundingSphere = mesh->boundingSphere();
        if (!camera.useFrustumCulling() || helpers::isVisibleInsideFrustum(boundingSphere, cameraFrustum)) {
            tracker.counter("draw-calls.renderer") += 1u;
            mesh->renderUnlit(commandBuffer, m_wireframePipelineHolder.pipelineLayout(), MESH_PUSH_CONSTANT_OFFSET);
        }
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Translucent pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.translucent-pass");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_translucentPipelineHolder.pipeline());

    // Draw all translucent meshes
    // @fixme We should sort the meshes
    // https://github.com/Breush/lava/issues/36
    for (auto& mesh : m_scene.meshes()) {
        if (!mesh->translucent() || (camera.vrAimed() && !mesh->vrRenderable())) continue;
        const auto& boundingSphere = mesh->boundingSphere();
        if (!camera.useFrustumCulling() || helpers::isVisibleInsideFrustum(boundingSphere, cameraFrustum)) {
            tracker.counter("draw-calls.renderer") += 1u;
            mesh->render(commandBuffer, m_translucentPipelineHolder.pipelineLayout(), MESH_PUSH_CONSTANT_OFFSET,
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
    return m_finalImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA + m_cameraId);
}

RenderImage ForwardRendererStage::depthRenderImage() const
{
    return m_depthImageHolder.renderImage(RenderImage::Impl::UUID_CONTEXT_CAMERA_DEPTH + m_cameraId);
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
    m_opaquePipelineHolder.add(m_scene.environmentDescriptorHolder().setLayout());
    m_opaquePipelineHolder.add(m_scene.materialDescriptorHolder().setLayout());
    m_opaquePipelineHolder.add(m_scene.lightsDescriptorHolder().setLayout());

    //----- Push constants

    m_opaquePipelineHolder.addPushConstantRange(sizeof(vulkan::MeshUbo));
    m_opaquePipelineHolder.addPushConstantRange(sizeof(vulkan::CameraUbo));

    //----- Rasterization

    m_opaquePipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().physicalDevice());
    m_opaquePipelineHolder.set(depthStencilAttachment);

    vulkan::PipelineHolder::ColorAttachment finalColorAttachment;
    finalColorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    finalColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    m_opaquePipelineHolder.add(finalColorAttachment);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(vulkan::Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(vulkan::Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(vulkan::Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(vulkan::Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(vulkan::Vertex, tangent)}};
    m_opaquePipelineHolder.set(vertexInput);
}

void ForwardRendererStage::initWireframePass()
{
    //----- Descriptor set layouts

    // @note Ordering is important
    m_wireframePipelineHolder.add(m_scene.environmentDescriptorHolder().setLayout());
    m_wireframePipelineHolder.add(m_scene.materialDescriptorHolder().setLayout());
    // @note Useful, so that the pipelines are all compatible
    m_wireframePipelineHolder.add(m_scene.lightsDescriptorHolder().setLayout());

    //----- Push constants

    m_wireframePipelineHolder.addPushConstantRange(sizeof(vulkan::MeshUbo));
    m_wireframePipelineHolder.addPushConstantRange(sizeof(vulkan::CameraUbo));

    //----- Rasterization

    m_wireframePipelineHolder.set(vk::CullModeFlagBits::eNone);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().physicalDevice());
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
    vertexInput.stride = sizeof(vulkan::UnlitVertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(vulkan::UnlitVertex, pos)}};
    m_wireframePipelineHolder.set(vertexInput);
}

void ForwardRendererStage::initTranslucentPass()
{
    // @note Translucent pass differs from opaque pass just by having
    // depth writing disabled and alpha blending enabled.

    //----- Descriptor set layouts

    // @note Ordering is important
    m_translucentPipelineHolder.add(m_scene.environmentDescriptorHolder().setLayout());
    m_translucentPipelineHolder.add(m_scene.materialDescriptorHolder().setLayout());
    m_translucentPipelineHolder.add(m_scene.lightsDescriptorHolder().setLayout());

    //----- Push constants

    m_translucentPipelineHolder.addPushConstantRange(sizeof(vulkan::MeshUbo));
    m_translucentPipelineHolder.addPushConstantRange(sizeof(vulkan::CameraUbo));

    //----- Rasterization

    m_translucentPipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vulkan::depthBufferFormat(m_scene.engine().physicalDevice());
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
    vertexInput.stride = sizeof(vulkan::Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(vulkan::Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(vulkan::Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(vulkan::Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(vulkan::Vertex, tangent)}};
    m_translucentPipelineHolder.set(vertexInput);
}

void ForwardRendererStage::updatePassShaders(bool firstTime)
{
    // @note We use the very same shaders for opaque and translucent meshes,
    // only the depth test and the color attachments differ.

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["USE_CAMERA_PUSH_CONSTANT"] = "1";
    moduleOptions.defines["USE_MESH_PUSH_CONSTANT"] = "1";
    moduleOptions.defines["MATERIAL_DESCRIPTOR_SET_INDEX"] = std::to_string(MATERIAL_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["LIGHTS_DESCRIPTOR_SET_INDEX"] = std::to_string(LIGHTS_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["ENVIRONMENT_DESCRIPTOR_SET_INDEX"] = std::to_string(ENVIRONMENT_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT"] = std::to_string(ENVIRONMENT_RADIANCE_MIP_LEVELS_COUNT);
    moduleOptions.defines["LIGHT_TYPE_POINT"] = std::to_string(static_cast<uint32_t>(LightType::Point));
    moduleOptions.defines["LIGHT_TYPE_DIRECTIONAL"] = std::to_string(static_cast<uint32_t>(LightType::Directional));
    moduleOptions.defines["MATERIAL_DATA_SIZE"] = std::to_string(MATERIAL_DATA_SIZE);
    moduleOptions.defines["MATERIAL_SAMPLERS_SIZE"] = std::to_string(MATERIAL_SAMPLERS_SIZE);
    moduleOptions.defines["G_BUFFER_DATA_SIZE"] = std::to_string(G_BUFFER_DATA_SIZE);
    if (firstTime) moduleOptions.updateCallback = [this]() { updatePassShaders(false); };

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/geometry.vert", moduleOptions);
    auto fragmentShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/renderers/forward/geometry.frag", moduleOptions);

    auto unlitVertexShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/geometry-unlit.vert", moduleOptions);
    auto unlitFragmentShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/geometry-unlit.frag", moduleOptions);

    m_opaquePipelineHolder.removeShaderStages();
    m_opaquePipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});
    m_opaquePipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

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
    auto depthFormat = vulkan::depthBufferFormat(m_scene.engine().physicalDevice());
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

    // Framebuffer
    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPassHolder.renderPass();
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    if (m_scene.engine().device().createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.forward-renderer-stage") << "Failed to create framebuffers." << std::endl;
    }
}
