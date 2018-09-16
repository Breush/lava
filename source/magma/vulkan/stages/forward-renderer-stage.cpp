#include "./forward-renderer-stage.hpp"

#include <lava/chamber/logger.hpp>
#include <lava/chamber/tracker.hpp>

#include "../../g-buffer-data.hpp"
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

ForwardRendererStage::ForwardRendererStage(RenderScene::Impl& scene)
    : m_scene(scene)
    , m_renderPassHolder(m_scene.engine())
    , m_opaquePipelineHolder(m_scene.engine())
    , m_translucentPipelineHolder(m_scene.engine())
    , m_finalImageHolder(m_scene.engine())
    , m_depthImageHolder(m_scene.engine())
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
    initTranslucentPass();

    //----- Render pass

    m_renderPassHolder.add(m_opaquePipelineHolder);
    m_renderPassHolder.add(m_translucentPipelineHolder);
    m_renderPassHolder.init();

    logger.log().tab(-1);
}

void ForwardRendererStage::update(vk::Extent2D extent, vk::PolygonMode polygonMode)
{
    m_extent = extent;

    m_opaquePipelineHolder.update(extent, polygonMode);
    m_translucentPipelineHolder.update(extent, polygonMode);

    createResources();
    createFramebuffers();
}

void ForwardRendererStage::render(vk::CommandBuffer commandBuffer)
{
    const auto& deviceHolder = m_scene.engine().deviceHolder();
    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer");

    //----- Prologue

    // @todo These clearValues could be returned by a method of RenderPassHolder,
    // making sure it has the right size and with default values already set
    // during construction.

    // Set render pass
    std::array<vk::ClearValue, 3> clearValues;
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

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.pass");

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_opaquePipelineHolder.pipeline());

    // Bind lights
    for (auto lightId = 0u; lightId < m_scene.lightsCount(); ++lightId) {
        m_scene.light(lightId).render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), LIGHTS_DESCRIPTOR_SET_INDEX);
    }

    // Set the camera
    auto& camera = m_scene.camera(m_cameraId);
    camera.render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), CAMERA_DESCRIPTOR_SET_INDEX);

    // Draw all opaque meshes
    for (auto& mesh : m_scene.meshes()) {
        tracker.counter("draw-calls.renderer") += 1u;
        mesh->interfaceImpl().render(commandBuffer, m_opaquePipelineHolder.pipelineLayout(), MESH_DESCRIPTOR_SET_INDEX,
                                     MATERIAL_DESCRIPTOR_SET_INDEX);
    }

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Translucent pass

    deviceHolder.debugBeginRegion(commandBuffer, "forward-renderer.pass");

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_translucentPipelineHolder.pipeline());

    // Draw all translucent meshes
    // @fixme In forward, we have to sort transparent meshes,
    // they are currently drawn as opaque too. (See #36)
    // But this pass is ready!

    deviceHolder.debugEndRegion(commandBuffer);

    //----- Epilogue

    commandBuffer.endRenderPass();

    deviceHolder.debugEndRegion(commandBuffer);
}

RenderImage ForwardRendererStage::renderImage() const
{
    RenderImage renderImage;
    renderImage.impl().uuid(RenderImage::Impl::UUID_CONTEXT_CAMERA + m_cameraId);
    renderImage.impl().view(m_finalImageHolder.view());
    renderImage.impl().layout(vk::ImageLayout::eColorAttachmentOptimal);
    return renderImage;
}

RenderImage ForwardRendererStage::depthRenderImage() const
{
    RenderImage renderImage;
    renderImage.impl().uuid(RenderImage::Impl::UUID_CONTEXT_CAMERA_DEPTH + m_cameraId);
    renderImage.impl().view(m_depthImageHolder.view());
    renderImage.impl().layout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
    return renderImage;
}

//----- Internal

void ForwardRendererStage::initOpaquePass()
{
    //----- Descriptor set layouts

    // @note Ordering is important
    m_opaquePipelineHolder.add(m_scene.cameraDescriptorHolder().setLayout());
    m_opaquePipelineHolder.add(m_scene.materialDescriptorHolder().setLayout());
    m_opaquePipelineHolder.add(m_scene.meshDescriptorHolder().setLayout());
    m_opaquePipelineHolder.add(m_scene.lightsDescriptorHolder().setLayout());

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

void ForwardRendererStage::initTranslucentPass()
{
    // @note Translucent pass differs from opaque pass just by having
    // no depth test and alpha blending enabled.

    //----- Descriptor set layouts

    // @note Ordering is important
    m_translucentPipelineHolder.add(m_scene.cameraDescriptorHolder().setLayout());
    m_translucentPipelineHolder.add(m_scene.materialDescriptorHolder().setLayout());
    m_translucentPipelineHolder.add(m_scene.meshDescriptorHolder().setLayout());
    m_translucentPipelineHolder.add(m_scene.lightsDescriptorHolder().setLayout());

    //----- Rasterization

    m_translucentPipelineHolder.set(vk::CullModeFlagBits::eBack);

    //----- Attachments

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
    moduleOptions.defines["CAMERA_DESCRIPTOR_SET_INDEX"] = std::to_string(CAMERA_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["MESH_DESCRIPTOR_SET_INDEX"] = std::to_string(MESH_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["MATERIAL_DESCRIPTOR_SET_INDEX"] = std::to_string(MATERIAL_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["LIGHTS_DESCRIPTOR_SET_INDEX"] = std::to_string(LIGHTS_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["LIGHT_TYPE_POINT"] = std::to_string(static_cast<uint32_t>(LightType::Point));
    moduleOptions.defines["LIGHT_TYPE_DIRECTIONAL"] = std::to_string(static_cast<uint32_t>(LightType::Directional));
    moduleOptions.defines["G_BUFFER_DATA_SIZE"] = std::to_string(G_BUFFER_DATA_SIZE);
    if (firstTime) moduleOptions.updateCallback = [this]() { updatePassShaders(false); };

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/geometry.vert", moduleOptions);
    auto fragmentShaderModule =
        m_scene.engine().shadersManager().module("./data/shaders/stages/renderers/forward/geometry.frag", moduleOptions);

    m_opaquePipelineHolder.removeShaderStages();
    m_opaquePipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});
    m_opaquePipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    m_translucentPipelineHolder.removeShaderStages();
    m_translucentPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});
    m_translucentPipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    if (!firstTime) {
        m_opaquePipelineHolder.update(m_extent);
        m_translucentPipelineHolder.update(m_extent);
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
