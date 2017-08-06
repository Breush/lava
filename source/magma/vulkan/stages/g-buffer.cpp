#include "./g-buffer.hpp"

#include <lava/chamber/logger.hpp>

#include "../buffer.hpp"
#include "../image.hpp"
#include "../render-engine-impl.hpp"
#include "../shader.hpp"
#include "../user-data-render.hpp"
#include "../vertex.hpp"

using namespace lava::magma;
using namespace lava::chamber;

GBuffer::GBuffer(RenderEngine::Impl& engine)
    : RenderStage(engine)
    , m_vertexShaderModule{m_engine.device().vk()}
    , m_fragmentShaderModule{m_engine.device().vk()}
    , m_normalImageHolder{m_engine.device(), m_engine.commandPool()}
    , m_albedoImageHolder{m_engine.device(), m_engine.commandPool()}
    , m_ormImageHolder{m_engine.device(), m_engine.commandPool()}
    , m_depthImageHolder{m_engine.device(), m_engine.commandPool()}
    , m_framebuffer{m_engine.device().vk()}
{
}

//----- RenderStage

void GBuffer::stageInit()
{
    logger.log() << "Initializing G-Buffer stage." << std::endl;
    logger.log().tab(1);

    // @cleanup HPP
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    //----- Shaders

    auto vertexShaderCode = vulkan::readGlslShaderFile("./data/shaders/stages/g-buffer.vert");
    vulkan::createShaderModule(vk_device, vertexShaderCode, m_vertexShaderModule);
    add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, m_vertexShaderModule, "main"});

    auto fragmentShaderCode = vulkan::readGlslShaderFile("./data/shaders/stages/g-buffer.frag");
    vulkan::createShaderModule(vk_device, fragmentShaderCode, m_fragmentShaderModule);
    add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, m_fragmentShaderModule, "main"});

    //----- Descriptor set layouts

    // @cleanup HPP vk::DescriptorSetLayout Should be made here anyway
    add(vk::DescriptorSetLayout(m_engine.cameraDescriptorSetLayout()));
    add(vk::DescriptorSetLayout(m_engine.materialDescriptorSetLayout()));
    add(vk::DescriptorSetLayout(m_engine.meshDescriptorSetLayout()));

    //----- Attachments

    ColorAttachment colorAttachment;
    colorAttachment.format = vk::Format::eR8G8B8A8Unorm;
    colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    add(colorAttachment); // Normal
    add(colorAttachment); // Albedo
    add(colorAttachment); // ORM

    DepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.format =
        static_cast<vk::Format>(vulkan::findDepthBufferFormat(device.physicalDevice())); // @cleanup HPP
    set(depthStencilAttachment);

    //---- Vertex input

    // @todo Should not be stored by ourself, but added through the interface
    // @cleanup HPP
    m_vertexInputBindingDescription = vulkan::Vertex::bindingDescription();
    auto attributeDescriptions = vulkan::Vertex::attributeDescriptions();
    for (auto& attribute : attributeDescriptions) {
        m_vertexInputAttributeDescriptions.emplace_back(attribute);
    }

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
    // @cleanup HPP
    auto depthFormat = vulkan::findDepthBufferFormat(m_engine.device().physicalDevice());
    m_depthImageHolder.create(vk::Format(depthFormat), m_extent, vk::ImageAspectFlagBits::eDepth);
}

void GBuffer::createFramebuffers()
{
    // @cleanup HPP
    const auto& vk_device = m_engine.device().vk();

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

    if (vk_device.createFramebuffer(&framebufferInfo, nullptr, m_framebuffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.g-buffer") << "Failed to create framebuffers." << std::endl;
    }
}
