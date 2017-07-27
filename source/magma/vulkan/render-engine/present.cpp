#include "./present.hpp"

#include <lava/chamber/logger.hpp>

#include "../buffer.hpp"
#include "../image.hpp"
#include "../render-engine-impl.hpp"
#include "../shader.hpp"
#include "../vertex.hpp"

namespace {
    struct Vertex {
        glm::vec2 position;
    };
}

using namespace lava::magma;
using namespace lava::chamber;

Present::Present(RenderEngine::Impl& engine)
    : IStage(engine)
    , m_descriptorPool{m_engine.device().vk()}
    , m_descriptorSetLayout{m_engine.device().vk()}
{
}

//----- IStage

void Present::init()
{
    logger.log() << "Initializing Present Stage." << std::endl;
    logger.log().tab(1);

    // @cleanup HPP
    const auto& vk_device = m_engine.device().vk();

    //----- Shaders

    auto vertexShaderCode = vulkan::readGlslShaderFile("./data/shaders/render-engine/present.vert");
    auto fragmentShaderCode = vulkan::readGlslShaderFile("./data/shaders/render-engine/present.frag");

    vulkan::createShaderModule(vk_device, vertexShaderCode, m_vertexShaderModule);
    vulkan::createShaderModule(vk_device, fragmentShaderCode, m_fragmentShaderModule);

    //----- Descriptor pool

    // @todo Should we really have so many pools?
    // Maybe just one in a central place.

    vk::DescriptorPoolSize poolSize;
    poolSize.type = vk::DescriptorType::eCombinedImageSampler;
    poolSize.descriptorCount = 1u;

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.poolSizeCount = 1u;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1u;

    if (vk_device.createDescriptorPool(&poolInfo, nullptr, m_descriptorPool.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.present") << "Failed to create descriptor pool." << std::endl;
    }

    //----- Descriptor set layout

    vk::DescriptorSetLayoutBinding layoutBinding;
    layoutBinding.binding = 0;
    layoutBinding.descriptorCount = 1;
    layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    layoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &layoutBinding;

    if (vk_device.createDescriptorSetLayout(&layoutInfo, nullptr, m_descriptorSetLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.present") << "Failed to create material descriptor set layout." << std::endl;
    }

    //----- Descriptor set

    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vk_device.allocateDescriptorSets(&allocInfo, &m_descriptorSet) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.present") << "Failed to create descriptor set." << std::endl;
    }

    logger.log().tab(-1);
}

void Present::update(const vk::Extent2D& extent)
{
    logger.log() << "Updating Present stage." << std::endl;
    logger.log().tab(1);

    IStage::update(extent);

    createRenderPass();
    createGraphicsPipeline();
    createResources();
    createFramebuffers();

    logger.log().tab(-1);
}

void Present::render(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex)
{
    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 1> clearValues;
    // clearValues[0] does not matter

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffers[frameIndex];
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    //----- Render

    // Bind pipeline
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

    // Draw
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
    commandBuffer.draw(6, 1, 0, 0);

    //----- Epilogue

    commandBuffer.endRenderPass();
}

//----- Internal

void Present::createRenderPass()
{
    // @cleanup HPP
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    // Present attachement
    vk::Format presentAttachmentFormat = vk::Format(m_engine.swapchain().imageFormat()); // @cleanup HPP
    vk::AttachmentReference presentAttachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription presentAttachment;
    presentAttachment.format = presentAttachmentFormat;
    presentAttachment.samples = vk::SampleCountFlagBits::e1;
    presentAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    presentAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    presentAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    presentAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    presentAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    std::array<vk::AttachmentReference, 1> colorAttachmentsRefs = {presentAttachmentRef};
    std::array<vk::AttachmentDescription, 1> attachments = {presentAttachment};

    // Subpass
    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = colorAttachmentsRefs.size();
    subpass.pColorAttachments = colorAttachmentsRefs.data();

    vk::SubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

    // The render pass indeed
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vk_device.createRenderPass(&renderPassInfo, nullptr, m_renderPass.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.present") << "Failed to create render pass." << std::endl;
    }
}

void Present::createGraphicsPipeline()
{

    // @cleanup HPP
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();

    // Pipeline layout
    // __Note__: Order IS important, as sets numbers in shader correspond to order of appearance in this list
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vk_device.createPipelineLayout(&pipelineLayoutInfo, nullptr, m_pipelineLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine.present") << "Failed to create pipeline layout." << std::endl;
    }

    updatePipeline();
}

void Present::createResources()
{
    // __Note__: Nothing.
}

void Present::createFramebuffers()
{
    // @cleanup HPP
    auto& device = m_engine.device();
    const auto& vk_device = device.vk();
    auto& swapchain = m_engine.swapchain();

    m_framebuffers.resize(swapchain.imageViews().size(), vulkan::Framebuffer{m_engine.device().vk()});

    for (size_t i = 0; i < swapchain.imageViews().size(); i++) {
        std::array<vk::ImageView, 1> attachments = {vk::ImageView(swapchain.imageViews()[i])};

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchain.extent().width;
        framebufferInfo.height = swapchain.extent().height;
        framebufferInfo.layers = 1;

        if (vk_device.createFramebuffer(&framebufferInfo, nullptr, m_framebuffers[i].replace()) != vk::Result::eSuccess) {
            logger.error("magma.vulkan.render-engine.present") << "Failed to create framebuffers." << std::endl;
        }
    }
}

void Present::shownImageView(const vk::ImageView& imageView, const vk::Sampler& sampler)
{
    // @cleanup HPP
    const auto& vk_device = m_engine.device().vk();

    vk::DescriptorImageInfo imageInfo;
    // @note Correspond to the final layout specified at previous pass
    imageInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = m_descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vk_device.updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}
