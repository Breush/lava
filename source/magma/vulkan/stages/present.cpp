#include "./present.hpp"

#include <lava/chamber/logger.hpp>

#include "../buffer.hpp"
#include "../image.hpp"
#include "../render-engine-impl.hpp"
#include "../shader.hpp"
#include "../vertex.hpp"

using namespace lava::magma;
using namespace lava::chamber;

Present::Present(RenderEngine::Impl& engine)
    : RenderStage(engine)
    , m_vertexShaderModule{engine.device()}
    , m_fragmentShaderModule{engine.device()}
    , m_descriptorPool{engine.device()}
    , m_descriptorSetLayout{engine.device()}
{
    updateCleverlySkipped(false); // As we're creating our framebuffers based on swapchain image views.
}

//----- RenderStage

void Present::stageInit()
{
    logger.log() << "Initializing Present Stage." << std::endl;
    logger.log().tab(1);

    if (!m_swapchainHolder) {
        logger.error("magma.vulkan.stages.present") << "No swapchain holder binded before initialization." << std::endl;
    }

    //----- Shaders

    auto vertexShaderCode = vulkan::readGlslShaderFile("./data/shaders/stages/present.vert");
    vulkan::createShaderModule(m_engine.device(), vertexShaderCode, m_vertexShaderModule);
    add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, m_vertexShaderModule, "main"});

    auto fragmentShaderCode = vulkan::readGlslShaderFile("./data/shaders/stages/present.frag");
    vulkan::createShaderModule(m_engine.device(), fragmentShaderCode, m_fragmentShaderModule);
    add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, m_fragmentShaderModule, "main"});

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

    if (m_engine.device().createDescriptorPool(&poolInfo, nullptr, m_descriptorPool.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.present") << "Failed to create descriptor pool." << std::endl;
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

    if (m_engine.device().createDescriptorSetLayout(&layoutInfo, nullptr, m_descriptorSetLayout.replace())
        != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.present") << "Failed to create material descriptor set layout." << std::endl;
    }

    add(m_descriptorSetLayout);

    //----- Descriptor set

    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

    if (m_engine.device().allocateDescriptorSets(&allocInfo, &m_descriptorSet) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.stages.present") << "Failed to create descriptor set." << std::endl;
    }

    logger.log().tab(-1);

    //----- Attachments

    // @cleanup HPP
    ColorAttachment presentColorAttachment;
    presentColorAttachment.format = m_swapchainHolder->imageFormat();
    presentColorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
    add(presentColorAttachment);
}

void Present::stageUpdate()
{
    logger.log() << "Updating Present stage." << std::endl;
    logger.log().tab(1);

    createResources();
    createFramebuffers();

    logger.log().tab(-1);
}

void Present::stageRender(const vk::CommandBuffer& commandBuffer)
{
    const auto frameIndex = m_swapchainHolder->currentIndex();

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

void Present::createResources()
{
    // __Note__: Nothing.
}

void Present::createFramebuffers()
{
    // @cleanup HPP
    auto& imageViews = m_swapchainHolder->imageViews();

    m_framebuffers.resize(imageViews.size(), vulkan::Framebuffer{m_engine.device()});

    for (size_t i = 0; i < imageViews.size(); i++) {
        std::array<vk::ImageView, 1> attachments = {imageViews[i]};

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_extent.width;
        framebufferInfo.height = m_extent.height;
        framebufferInfo.layers = 1;

        if (m_engine.device().createFramebuffer(&framebufferInfo, nullptr, m_framebuffers[i].replace()) != vk::Result::eSuccess) {
            logger.error("magma.vulkan.stages.present") << "Failed to create framebuffers." << std::endl;
        }
    }
}

void Present::bindSwapchainHolder(vulkan::SwapchainHolder& swapchainHolder)
{
    m_swapchainHolder = &swapchainHolder;
}

void Present::imageView(const vk::ImageView& imageView, const vk::Sampler& sampler)
{
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

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}
