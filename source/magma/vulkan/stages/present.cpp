#include "./present.hpp"

#include "../helpers/descriptor.hpp"
#include "../holders/swapchain-holder.hpp"
#include "../render-engine-impl.hpp"

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
    m_viewInfos.reserve(MAX_VIEW_COUNT);
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
                     {sizeof(ViewUbo), {sizeof(ViewportUbo), MAX_VIEW_COUNT}});

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

void Present::update(const vk::Extent2D& extent)
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
    commandBuffer.draw(3, 1, 0, 0);

    //----- Epilogue

    commandBuffer.endRenderPass();
}

void Present::bindSwapchainHolder(const vulkan::SwapchainHolder& swapchainHolder)
{
    m_swapchainHolder = &swapchainHolder;
}

uint32_t Present::addView(vk::ImageView imageView, vk::ImageLayout imageLayout, vk::Sampler sampler, Viewport viewport, uint32_t channelCount)
{
    if (m_viewInfos.size() >= MAX_VIEW_COUNT) {
        logger.warning("magma.vulkan.stages.present")
            << "Cannot add another view. "
            << "Maximum view count is currently set to " << MAX_VIEW_COUNT << "." << std::endl;
        return -1u;
    }

    // Adding the view
    const uint32_t viewId = m_nextViewId;
    m_nextViewId += 1u;

    ViewInfo viewInfo;
    viewInfo.id = viewId;
    viewInfo.viewport = viewport;
    viewInfo.imageView = imageView;
    viewInfo.imageLayout = imageLayout;
    viewInfo.sampler = sampler;
    viewInfo.channelCount = channelCount;
    m_viewInfos.emplace(viewId, viewInfo);

    updateUbos();

    return viewId;
}

void Present::removeView(uint32_t viewId)
{
    auto viewInfoIt = m_viewInfos.find(viewId);

    if (viewInfoIt == m_viewInfos.end()) {
        logger.warning("magma.vulkan.stages.present") << "Cannot remove view " << viewId << " which does not exist." << std::endl;
        return;
    }

    m_viewInfos.erase(viewInfoIt);
    updateUbos();
}

void Present::updateView(uint32_t viewId, vk::ImageView imageView, vk::ImageLayout imageLayout, vk::Sampler sampler)
{
    m_viewInfos.at(viewId).imageView = imageView;
    m_viewInfos.at(viewId).imageLayout = imageLayout;
    m_viewInfos.at(viewId).sampler = sampler;

    updateUbos();
}

// ----- Internal

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

void Present::updateUbos()
{
    // @todo Better have uboDirty and update(dt)?
    m_engine.device().waitIdle();

    ViewUbo viewUbo;
    viewUbo.count = m_viewInfos.size();
    m_uboHolder.copy(0, viewUbo);

    // Sort based on depth
    std::vector<ViewInfo> sortedViewInfos;
    sortedViewInfos.reserve(m_viewInfos.size());
    for (const auto& viewInfoPair : m_viewInfos) {
        sortedViewInfos.emplace_back(viewInfoPair.second);
    }
    std::sort(sortedViewInfos.begin(), sortedViewInfos.end(), [](const ViewInfo& viewInfo1, const ViewInfo& viewInfo2) {
        return viewInfo1.viewport.depth > viewInfo2.viewport.depth;
    });

    // Now set ubos
    for (auto i = 0u; i < sortedViewInfos.size(); ++i) {
        const auto& viewInfo = sortedViewInfos[i];
        ViewportUbo viewportUbo;
        viewportUbo.x = viewInfo.viewport.x;
        viewportUbo.y = viewInfo.viewport.y;
        viewportUbo.width = viewInfo.viewport.width;
        viewportUbo.height = viewInfo.viewport.height;
        viewportUbo.channelCount = viewInfo.channelCount;

        m_uboHolder.copy(1, viewportUbo, i);
        vulkan::updateDescriptorSet(m_engine.device(), m_descriptorSet, viewInfo.imageView, viewInfo.sampler, viewInfo.imageLayout, 2u, i);
    }
}
