#include "./present.hpp"

#include "../helpers/descriptor.hpp"
#include "../holders/swapchain-holder.hpp"
#include "../render-engine-impl.hpp"
#include "../vertex.hpp"

using namespace lava::magma;
using namespace lava::chamber;

namespace {
    constexpr const auto MAX_VIEW_COUNT = 8u;
}

Present::Present(RenderEngine::Impl& engine)
    : m_engine(engine)
    , m_renderPassHolder(engine)
    , m_pipelineHolder(engine)
    , m_descriptorHolder(engine)
    , m_uboHolder(engine)
{
    m_viewports.reserve(MAX_VIEW_COUNT);
}

void Present::init()
{
    logger.info("magma.vulkan.stages.present") << "Initializing." << std::endl;
    logger.log().tab(1);

    if (!m_swapchainHolder) {
        logger.error("magma.vulkan.stages.present") << "No swapchain holder binded before initialization." << std::endl;
    }

    if (m_descriptorSet) {
        logger.warning("magma.vulkan.stages.present") << "Already initialized." << std::endl;
        return;
    }

    //----- Shaders

    auto vertexShaderModule = m_engine.shadersManager().module("./data/shaders/stages/fullscreen.vert");
    m_pipelineHolder.add({vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"});

    ShadersManager::ModuleOptions moduleOptions;
    moduleOptions.defines["MAX_VIEW_COUNT"] = std::to_string(MAX_VIEW_COUNT);
    auto fragmentShaderModule = m_engine.shadersManager().module("./data/shaders/stages/present.frag", moduleOptions);
    m_pipelineHolder.add(
        {vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"});

    //----- Descriptors

    m_descriptorHolder.uniformBufferSizes({1, MAX_VIEW_COUNT});
    m_descriptorHolder.combinedImageSamplerSizes({MAX_VIEW_COUNT});
    m_descriptorHolder.init(1, vk::ShaderStageFlagBits::eFragment);
    m_pipelineHolder.add(m_descriptorHolder.setLayout());

    m_descriptorSet = m_descriptorHolder.allocateSet("present");

    // Mock-up samplers
    // @fixme Why would you want dummy sampler here?
    // Just make one that has no fancy stuff in it...
    const auto imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    const auto& dummySampler = m_engine.dummySampler();
    const auto& dummyImageView = m_engine.dummyImageView();
    for (auto i = 0u; i < MAX_VIEW_COUNT; ++i) {
        vulkan::updateDescriptorSet(m_engine.device(), m_descriptorSet, dummyImageView, dummySampler, imageLayout, 2u, i);
    }

    //----- Uniform buffers

    m_uboHolder.init(m_descriptorSet, m_descriptorHolder.uniformBufferBindingOffset(),
                     {sizeof(uint32_t), {sizeof(Viewport), MAX_VIEW_COUNT}});

    //----- Attachments

    vulkan::PipelineHolder::ColorAttachment presentColorAttachment;
    presentColorAttachment.format = m_swapchainHolder->imageFormat();
    presentColorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
    m_pipelineHolder.add(presentColorAttachment);

    //----- Render pass

    m_renderPassHolder.add(m_pipelineHolder);
    m_renderPassHolder.init();

    m_pipelineHolder.init(0u);

    logger.log().tab(-1);
}

void Present::update(vk::Extent2D extent)
{
    m_extent = extent;

    m_pipelineHolder.update(extent);
    createFramebuffers();
}

void Present::render(vk::CommandBuffer commandBuffer)
{
    const auto frameIndex = m_swapchainHolder->currentIndex();

    //----- Prologue

    // Set render pass
    std::array<vk::ClearValue, 1> clearValues;
    // clearValues[0] does not matter

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = m_renderPassHolder.renderPass();
    renderPassInfo.framebuffer = m_framebuffers[frameIndex];
    renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderPassInfo.renderArea.extent = m_extent;
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    //----- Render

    // Bind pipeline
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelineHolder.pipeline());

    // Draw
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineHolder.pipelineLayout(), 0, 1, &m_descriptorSet,
                                     0, nullptr);
    commandBuffer.draw(6, 1, 0, 0);

    //----- Epilogue

    commandBuffer.endRenderPass();
}

//----- Internal

void Present::createFramebuffers()
{
    auto& imageViews = m_swapchainHolder->imageViews();

    m_framebuffers.resize(imageViews.size(), vulkan::Framebuffer{m_engine.device()});

    for (size_t i = 0; i < imageViews.size(); i++) {
        std::array<vk::ImageView, 1> attachments = {imageViews[i]};

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = m_renderPassHolder.renderPass();
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

void Present::bindSwapchainHolder(const vulkan::SwapchainHolder& swapchainHolder)
{
    m_swapchainHolder = &swapchainHolder;
}

uint32_t Present::addView(vk::ImageView imageView, vk::ImageLayout imageLayout, vk::Sampler sampler, Viewport viewport)
{
    if (m_viewports.size() >= MAX_VIEW_COUNT) {
        logger.warning("magma.vulkan.stages.present")
            << "Cannot add another view. "
            << "Maximum view count is currently set to " << MAX_VIEW_COUNT << "." << std::endl;
        return -1u;
    }

    // Adding the view
    const uint32_t viewId = m_viewports.size();
    m_viewports.emplace_back(viewport);

    //---- Descriptors

    // Views count update
    const uint32_t viewCount = m_viewports.size();
    m_uboHolder.copy(0, viewCount);

    // Viewports descriptor
    m_uboHolder.copy(1, m_viewports.back(), viewId);

    // Samplers descriptor
    updateView(viewId, imageView, imageLayout, sampler);

    return viewId;
}

void Present::removeView(uint32_t viewId)
{
    // @todo Handle views individually, generating a really unique id
    // that has nothing to do with internal storage.
    if (viewId != m_viewports.size() - 1u) {
        logger.warning("magma.vulkan.stages.present")
            << "Currently unable to remove a view that is not the last one added." << std::endl;
        return;
    }

    m_viewports.erase(std::begin(m_viewports) + viewId);

    // Views count update
    const uint32_t viewCount = m_viewports.size();
    m_uboHolder.copy(0, viewCount);
}

void Present::updateView(uint32_t viewId, vk::ImageView imageView, vk::ImageLayout imageLayout, vk::Sampler sampler)
{
    // @todo Also add greyscale image and linear depth...

    vulkan::updateDescriptorSet(m_engine.device(), m_descriptorSet, imageView, sampler, imageLayout, 2u, viewId);
}
