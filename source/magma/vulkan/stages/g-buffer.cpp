#include "./g-buffer.hpp"

#include <lava/chamber/logger.hpp>

#include "../cameras/i-camera-impl.hpp"
#include "../meshes/i-mesh-impl.hpp"
#include "../render-engine-impl.hpp"
#include "../render-scenes/render-scene-impl.hpp"
#include "../vertex.hpp"

namespace {
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

GBuffer::GBuffer(RenderScene::Impl& scene)
    : RenderStage(scene.engine())
    , m_scene(scene)
    , m_normalImageHolder{m_engine}
    , m_albedoImageHolder{m_engine}
    , m_ormImageHolder{m_engine}
    , m_depthImageHolder{m_engine}
    , m_framebuffer{m_engine.device()}
{
}

void GBuffer::init(uint32_t cameraId)
{
    m_cameraId = cameraId;

    RenderStage::init();
}

//----- RenderStage

void GBuffer::stageInit()
{
    logger.info("magma.vulkan.stages.g-buffer") << "Initializing." << std::endl;
    logger.log().tab(1);

    if (m_cameraId == -1u) {
        logger.error("magma.vulkan.stages.g-buffer") << "Initialized with no cameraId provided." << std::endl;
    }

    //----- Shaders

    m_vertexShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/g-buffer.vert");
    add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, m_vertexShaderModule, "main"});

    m_fragmentShaderModule = m_scene.engine().shadersManager().module("./data/shaders/stages/g-buffer.frag");
    add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, m_fragmentShaderModule, "main"});

    //----- Descriptor set layouts

    // @note Ordering is important
    add(m_scene.cameraDescriptorHolder().setLayout());
    add(m_scene.materialDescriptorHolder().setLayout());
    add(m_scene.meshDescriptorHolder().setLayout());

    //----- Attachments

    ColorAttachment colorAttachment;
    colorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    add(colorAttachment); // Normal
    add(colorAttachment); // Albedo
    add(colorAttachment); // ORM

    DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format = findDepthBufferFormat(m_engine.physicalDevice());
    set(depthStencilAttachment);

    //---- Vertex input

    // @todo Should not be stored by ourself, but added through the interface
    m_vertexInputBindingDescription.binding = 0;
    m_vertexInputBindingDescription.stride = sizeof(vulkan::Vertex);
    m_vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

    m_vertexInputAttributeDescriptions.resize(4);

    m_vertexInputAttributeDescriptions[0].binding = 0;
    m_vertexInputAttributeDescriptions[0].location = 0;
    m_vertexInputAttributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
    m_vertexInputAttributeDescriptions[0].offset = offsetof(vulkan::Vertex, pos);

    m_vertexInputAttributeDescriptions[1].binding = 0;
    m_vertexInputAttributeDescriptions[1].location = 1;
    m_vertexInputAttributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
    m_vertexInputAttributeDescriptions[1].offset = offsetof(vulkan::Vertex, uv);

    m_vertexInputAttributeDescriptions[2].binding = 0;
    m_vertexInputAttributeDescriptions[2].location = 2;
    m_vertexInputAttributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
    m_vertexInputAttributeDescriptions[2].offset = offsetof(vulkan::Vertex, normal);

    m_vertexInputAttributeDescriptions[3].binding = 0;
    m_vertexInputAttributeDescriptions[3].location = 3;
    m_vertexInputAttributeDescriptions[3].format = vk::Format::eR32G32B32A32Sfloat;
    m_vertexInputAttributeDescriptions[3].offset = offsetof(vulkan::Vertex, tangent);

    logger.log().tab(-1);
}

void GBuffer::stageUpdate()
{
    createResources();
    createFramebuffers();
}

void GBuffer::stageRender(const vk::CommandBuffer& commandBuffer)
{
    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 4> clearValues;
    clearValues[0].color = std::array<float, 4>{0.5f, 0.5f, 0.5f, 1.f};
    clearValues[1].color = std::array<float, 4>{0.2f, 0.6f, 0.4f, 1.f}; // @todo Allow to be configure
    clearValues[2].color = std::array<float, 4>{1.f, 0.f, 0.f, 1.f};
    clearValues[3].depthStencil = vk::ClearDepthStencilValue{1.f, 0u};

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    // Bind pipeline
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

    //----- Render

    // Set the camera
    auto& camera = m_scene.camera(m_cameraId);
    camera.render(commandBuffer, m_pipelineLayout, CAMERA_DESCRIPTOR_SET_INDEX);

    // Draw all opaque meshes
    for (auto& mesh : m_scene.meshes()) {
        mesh->interfaceImpl().render(commandBuffer, m_pipelineLayout, MESH_DESCRIPTOR_SET_INDEX);
    }

    //----- Epilogue

    commandBuffer.endRenderPass();
}

//----- Internal

vk::PipelineVertexInputStateCreateInfo GBuffer::pipelineVertexInputStateCreateInfo()
{
    vk::PipelineVertexInputStateCreateInfo vertexInputState;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &m_vertexInputBindingDescription;
    vertexInputState.vertexAttributeDescriptionCount = m_vertexInputAttributeDescriptions.size();
    vertexInputState.pVertexAttributeDescriptions = m_vertexInputAttributeDescriptions.data();

    return vertexInputState;
}

vk::PipelineRasterizationStateCreateInfo GBuffer::pipelineRasterizationStateCreateInfo()
{
    vk::PipelineRasterizationStateCreateInfo rasterizationState;
    rasterizationState.lineWidth = 1.f;
    rasterizationState.cullMode = vk::CullModeFlagBits::eBack;
    rasterizationState.frontFace = vk::FrontFace::eCounterClockwise;

    return rasterizationState;
}

void GBuffer::createResources()
{
    // Normal
    auto normalFormat = vk::Format::eR8G8B8A8Unorm;
    m_normalImageHolder.create(normalFormat, m_extent, vk::ImageAspectFlagBits::eColor);

    // Albedo
    auto albedoFormat = vk::Format::eR8G8B8A8Unorm;
    m_albedoImageHolder.create(albedoFormat, m_extent, vk::ImageAspectFlagBits::eColor);

    // ORM
    auto ormFormat = vk::Format::eR8G8B8A8Unorm;
    m_ormImageHolder.create(ormFormat, m_extent, vk::ImageAspectFlagBits::eColor);

    // Depth
    auto depthFormat = findDepthBufferFormat(m_engine.physicalDevice());
    m_depthImageHolder.create(depthFormat, m_extent, vk::ImageAspectFlagBits::eDepth);
}

void GBuffer::createFramebuffers()
{
    // Framebuffer
    std::array<vk::ImageView, 4> attachments = {m_normalImageHolder.view(), m_albedoImageHolder.view(), m_ormImageHolder.view(),
                                                m_depthImageHolder.view()};

    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    if (m_engine.device().createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.g-buffer") << "Failed to create framebuffers." << std::endl;
    }
}
