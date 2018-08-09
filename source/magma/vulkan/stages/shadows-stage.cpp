#include "./shadows-stage.hpp"

#include <lava/chamber/logger.hpp>

#include "../lights/i-light-impl.hpp"
#include "../meshes/i-mesh-impl.hpp"
#include "../render-engine-impl.hpp"
#include "../render-image-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../vertex.hpp"

using namespace lava::magma;
using namespace lava::chamber;

ShadowsStage::ShadowsStage(RenderScene::Impl& scene)
    : m_scene(scene)
    , m_renderPassHolder(m_scene.engine())
    , m_pipelineHolder(m_scene.engine())
    , m_depthImageHolder(scene.engine())
    , m_framebuffer(m_scene.engine().device())
{
}

void ShadowsStage::init(uint lightId)
{
    m_lightId = lightId;

    logger.info("magma.vulkan.stages.shadows-stage") << "Initializing." << std::endl;
    logger.log().tab(1);

    initPass();

    //----- Render pass

    m_renderPassHolder.add(m_pipelineHolder);
    m_renderPassHolder.init();

    logger.log().tab(-1);
}

void ShadowsStage::update(vk::Extent2D extent)
{
    m_extent = extent;

    m_pipelineHolder.update(extent);

    createResources();
    createFramebuffers();
}

void ShadowsStage::render(vk::CommandBuffer commandBuffer)
{
    const auto& deviceHolder = m_scene.engine().deviceHolder();
    deviceHolder.debugMarkerBeginRegion(commandBuffer, "ShadowsStage");

    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 1> clearValues;
    clearValues[0].depthStencil = vk::ClearDepthStencilValue{2e23, 0u};

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPassHolder.renderPass();
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    //----- Pass

    deviceHolder.debugMarkerBeginRegion(commandBuffer, "Pass");

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelineHolder.pipeline());

    auto& light = m_scene.light(m_lightId);
    light.renderShadows(commandBuffer, m_pipelineHolder.pipelineLayout(), SHADOWS_LIGHT_DESCRIPTOR_SET_INDEX);

    // Draw all meshes
    for (auto& mesh : m_scene.meshes()) {
        if (!mesh->canCastShadows()) continue;
        mesh->interfaceImpl().render(commandBuffer, m_pipelineHolder.pipelineLayout(), MESH_DESCRIPTOR_SET_INDEX, -1u);
    }

    // Draw
    commandBuffer.draw(6, 1, 0, 0);

    deviceHolder.debugMarkerEndRegion(commandBuffer);

    //----- Epilogue

    commandBuffer.endRenderPass();

    deviceHolder.debugMarkerEndRegion(commandBuffer);
}

RenderImage ShadowsStage::renderImage() const
{
    RenderImage renderImage;
    renderImage.impl().view(m_depthImageHolder.view());
    renderImage.impl().layout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
    return renderImage;
}

//----- Internal

void ShadowsStage::initPass()
{
    //----- Shaders

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["SHADOWS_LIGHT_DESCRIPTOR_SET_INDEX"] = std::to_string(SHADOWS_LIGHT_DESCRIPTOR_SET_INDEX);
    moduleOptions.defines["MESH_DESCRIPTOR_SET_INDEX"] = std::to_string(MESH_DESCRIPTOR_SET_INDEX);

    vk::PipelineShaderStageCreateFlags shaderStageCreateFlags;
    auto vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/shadows.vert", moduleOptions);
    m_pipelineHolder.add({shaderStageCreateFlags, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    //----- Descriptor set layouts

    // @note Ordering is important
    m_pipelineHolder.add(m_scene.lightDescriptorHolder().setLayout());
    m_pipelineHolder.add(m_scene.meshDescriptorHolder().setLayout());

    //----- Attachments

    // @todo Ensure that format is supported, and maybe let this be configurable
    vulkan::PipelineHolder::DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = vk::Format::eD16Unorm;
    m_pipelineHolder.set(depthStencilAttachment);

    //----- Rasterization

    m_pipelineHolder.set(vk::CullModeFlagBits::eBack);

    //---- Vertex input

    vulkan::PipelineHolder::VertexInput vertexInput;
    vertexInput.stride = sizeof(vulkan::Vertex);
    vertexInput.attributes = {{vk::Format::eR32G32B32Sfloat, offsetof(vulkan::Vertex, pos)},
                              {vk::Format::eR32G32Sfloat, offsetof(vulkan::Vertex, uv)},
                              {vk::Format::eR32G32B32Sfloat, offsetof(vulkan::Vertex, normal)},
                              {vk::Format::eR32G32B32A32Sfloat, offsetof(vulkan::Vertex, tangent)}};
    m_pipelineHolder.set(vertexInput);
}

void ShadowsStage::createResources()
{
    // Depth
    auto depthFormat = vk::Format::eD16Unorm;
    m_depthImageHolder.create(depthFormat, m_extent, vk::ImageAspectFlagBits::eDepth);
}

void ShadowsStage::createFramebuffers()
{
    // Framebuffer
    std::array<vk::ImageView, 1> attachments = {m_depthImageHolder.view()};

    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPassHolder.renderPass();
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    if (m_scene.engine().device().createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.shadows-stage") << "Failed to create framebuffers." << std::endl;
    }
}
