#include "./g-buffer.hpp"

#include <lava/chamber/logger.hpp>

#include "../helpers/shader.hpp"
#include "../render-engine-impl.hpp"
#include "../user-data-render.hpp"
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

GBuffer::GBuffer(RenderEngine::Impl& engine)
    : RenderStage(engine)
    , m_vertexShaderModule{engine.device()}
    , m_fragmentShaderModule{engine.device()}
    , m_normalImageHolder{engine}
    , m_albedoImageHolder{engine}
    , m_ormImageHolder{engine}
    , m_depthImageHolder{engine}
    , m_cameraDescriptorHolder{engine}
    , m_materialDescriptorHolder{engine}
    , m_meshDescriptorHolder{engine}
    , m_framebuffer{engine.device()}
{
}

//----- RenderStage

void GBuffer::stageInit()
{
    logger.log() << "Initializing G-Buffer stage." << std::endl;
    logger.log().tab(1);

    //----- Shaders

    auto vertexShaderCode = vulkan::readGlslShaderFile("./data/shaders/stages/g-buffer.vert");
    m_vertexShaderModule = vulkan::createShaderModule(m_engine.device(), vertexShaderCode);
    add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, m_vertexShaderModule, "main"});

    auto fragmentShaderCode = vulkan::readGlslShaderFile("./data/shaders/stages/g-buffer.frag");
    m_fragmentShaderModule = vulkan::createShaderModule(m_engine.device(), fragmentShaderCode);
    add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, m_fragmentShaderModule, "main"});

    //----- Descriptor set layouts

    m_cameraDescriptorHolder.init(1, 0, 1, vk::ShaderStageFlagBits::eVertex);
    m_materialDescriptorHolder.init(1, 3, 128, vk::ShaderStageFlagBits::eFragment);
    m_meshDescriptorHolder.init(1, 0, 128, vk::ShaderStageFlagBits::eVertex);

    // @note Ordering is important
    add(m_cameraDescriptorHolder.setLayout());
    add(m_materialDescriptorHolder.setLayout());
    add(m_meshDescriptorHolder.setLayout());

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
    logger.log() << "Updating G-Buffer stage." << std::endl;
    logger.log().tab(1);

    createResources();
    createFramebuffers();

    logger.log().tab(-1);
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

    UserDataRenderIn userData;
    userData.commandBuffer = &commandBuffer;
    userData.pipelineLayout = &m_pipelineLayout;

    // Draw all opaque meshes
    for (auto& camera : m_engine.cameras()) {
        camera->render(&userData);
        for (auto& mesh : m_engine.meshes()) {
            mesh->render(&userData);
        }

        // @todo Handle multiple cameras?
        // -> Probably not
        break;
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
