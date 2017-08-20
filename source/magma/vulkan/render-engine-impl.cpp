#include "./render-engine-impl.hpp"

#include <chrono>
#include <lava/chamber/logger.hpp>
#include <lava/magma/interfaces/render-target.hpp>

#include "./helpers/queue.hpp"
#include "./holders/swapchain-holder.hpp"
#include "./render-scenes/i-render-scene-impl.hpp"
#include "./wrappers.hpp"

using namespace lava::magma;
using namespace lava::chamber;

RenderEngine::Impl::Impl()
{
    initVulkan();
}

RenderEngine::Impl::~Impl()
{
    device().waitIdle();
}

void RenderEngine::Impl::draw()
{
    if (m_renderTargetBundles.size() != 1u) {
        logger.error("magma.vulkan.render-engine") << "No or too many render targets added during draw." << std::endl;
    }

    auto& renderTargetBundle = m_renderTargetBundles[0];
    auto& renderTarget = *renderTargetBundle.renderTarget;
    const auto& data = renderTargetBundle.data();

    renderTarget.prepare();

    // Record command buffer each frame
    device().waitIdle(); // @todo Better wait for a fence on the queue
    auto& commandBuffer = recordCommandBuffer(0, data.swapchainHolder.currentIndex());

    // Submit it to the queue
    vk::Semaphore waitSemaphores[] = {data.swapchainHolder.imageAvailableSemaphore()};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submitInfo;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderFinishedSemaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (graphicsQueue().submit(1, &submitInfo, nullptr) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.layer") << "Failed to submit draw command buffer." << std::endl;
    }

    InDataRenderTargetDraw drawData{m_renderFinishedSemaphore};
    renderTarget.draw(&drawData);
}

void RenderEngine::Impl::update()
{
}

// @todo Use viewport and renderSceneCameraIndex
uint32_t RenderEngine::Impl::addView(IRenderScene& renderScene, uint32_t /*renderSceneCameraIndex*/, IRenderTarget& renderTarget,
                                     Viewport /*viewport*/)
{
    const auto& renderSceneImpl = renderScene.interfaceImpl();
    const auto& dataRenderTarget = *reinterpret_cast<const DataRenderTarget*>(renderTarget.data());

    m_renderViews.emplace_back();
    auto& renderView = m_renderViews.back();
    renderView.renderScene = &renderScene;
    renderView.renderTarget = &renderTarget;
    renderView.presentStage = std::make_unique<Present>(*this);
    renderView.presentStage->bindSwapchainHolder(dataRenderTarget.swapchainHolder);
    renderView.presentStage->init();
    renderView.presentStage->update(dataRenderTarget.swapchainHolder.extent());
    renderView.presentStage->imageView(renderSceneImpl.imageView(), m_dummySampler);

    return m_renderViews.size() - 1u;
}

//----- Adders

void RenderEngine::Impl::add(std::unique_ptr<IRenderScene>&& renderScene)
{
    logger.info("magma.vulkan.render-engine") << "Adding render scene " << m_renderScenes.size() << "." << std::endl;
    logger.log().tab(1);

    // If no device yet, the scene initialization will be postponed
    // until it is created.
    if (m_deviceHolder.device()) {
        renderScene->init();
    }

    m_renderScenes.emplace_back(std::move(renderScene));

    logger.log().tab(-1);
}

void RenderEngine::Impl::add(std::unique_ptr<IRenderTarget>&& renderTarget)
{
    logger.info("magma.vulkan.render-engine") << "Adding render target " << m_renderTargetBundles.size() << "." << std::endl;
    logger.log().tab(1);

    const auto& data = *reinterpret_cast<const DataRenderTarget*>(renderTarget->data());

    // @todo This is probably not the right thing to do.
    // However - what can be the solution?
    // Device creation requires a surface, which requires a windowHandle.
    // Thus, no window => unable to init the device.
    // So we init what's left of the engine write here, when adding a renderTarget.
    {
        initVulkanDevice(data.surface); // As the render target below must have a valid device.
    }

    InDataRenderTargetInit dataRenderTargetInit;
    dataRenderTargetInit.id = m_renderTargetBundles.size();
    renderTarget->init(&dataRenderTargetInit);

    m_renderTargetBundles.emplace_back();
    auto& renderTargetBundle = m_renderTargetBundles.back();
    renderTargetBundle.renderTarget = std::move(renderTarget);

    createCommandBuffers(dataRenderTargetInit.id);

    logger.log().tab(-1);
}

void RenderEngine::Impl::updateRenderTarget(uint32_t renderTargetId)
{
    if (renderTargetId > m_renderTargetBundles.size()) {
        logger.warning("magma.vulkan.render-engine")
            << "Updating render target with invalid id: " << renderTargetId << "." << std::endl;
        return;
    }

    logger.info("magma.vulkan.render-engine") << "Updating render target " << renderTargetId << "." << std::endl;
    logger.log().tab(1);

    auto& renderTargetBundle = m_renderTargetBundles[renderTargetId];
    const auto& data = renderTargetBundle.data();
    const auto* renderTarget = renderTargetBundle.renderTarget.get();

    for (auto& renderView : m_renderViews) {
        if (renderView.renderTarget == renderTarget) {
            renderView.presentStage->update(data.swapchainHolder.extent());
        }
    }

    logger.log().tab(-1);
}

void RenderEngine::Impl::createCommandPool(vk::SurfaceKHR surface)
{
    logger.info("magma.vulkan.render-engine") << "Creating command pool." << std::endl;

    auto queueFamilyIndices = vulkan::findQueueFamilies(physicalDevice(), surface);

    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    if (device().createCommandPool(&poolInfo, nullptr, m_commandPool.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine") << "Failed to create command pool." << std::endl;
    }
}

void RenderEngine::Impl::createDummyTextures()
{
    logger.info("magma.vulkan.render-engine") << "Creating dummy textures." << std::endl;

    // Plain white
    std::vector<uint8_t> dummyData = {0xFF, 0xFF, 0xFF, 0xFF};
    m_dummyImageHolder.setup(dummyData, 1, 1, 4);

    // Flat blue
    dummyData = {0x80, 0x80, 0xFF, 0xFF};
    m_dummyNormalImageHolder.setup(dummyData, 1, 1, 4);

    // Sampler
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = true;
    samplerInfo.maxAnisotropy = 16; // Over 16 is useless, but lower that for better performances
    samplerInfo.unnormalizedCoordinates = false;
    samplerInfo.compareEnable = false;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

    if (device().createSampler(&samplerInfo, nullptr, m_dummySampler.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.texture-sampler") << "Failed to texture sampler." << std::endl;
    }
}

vk::CommandBuffer& RenderEngine::Impl::recordCommandBuffer(uint32_t renderTargetIndex, uint32_t bufferIndex)
{

    auto& renderTargetBundle = m_renderTargetBundles[renderTargetIndex];

    if (bufferIndex >= renderTargetBundle.commandBuffers.size()) {
        logger.error("magma.vulkan.render-engine")
            << "Invalid bufferIndex during command buffers recording (" << bufferIndex << ") that should have been between 0 and "
            << renderTargetBundle.commandBuffers.size() - 1u << "." << std::endl;
    }

    auto& commandBuffer = renderTargetBundle.commandBuffers[bufferIndex];

    //----- Prologue

    vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eSimultaneousUse};
    commandBuffer.begin(&beginInfo);

    //----- Render

    // @todo The scenes should know which cameras to render at any time.
    // And render only those.
    for (auto& renderScene : m_renderScenes) {
        renderScene->interfaceImpl().render(commandBuffer);
    }

    for (auto& renderView : m_renderViews) {
        renderView.presentStage->render(commandBuffer);
    }

    //----- Epilogue

    commandBuffer.end();

    return commandBuffer;
}

void RenderEngine::Impl::createCommandBuffers(uint32_t renderTargetIndex)
{
    logger.log() << "Creating command buffers for render target " << renderTargetIndex << "." << std::endl;

    auto& renderTargetBundle = m_renderTargetBundles[renderTargetIndex];
    auto& commandBuffers = renderTargetBundle.commandBuffers;
    auto& swapchain = renderTargetBundle.data().swapchainHolder;

    // Free previous command buffers if any
    if (commandBuffers.size() > 0) {
        device().freeCommandBuffers(m_commandPool, commandBuffers.size(), commandBuffers.data());
    }

    // Allocate them all
    commandBuffers.resize(swapchain.imagesCount());

    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.commandPool = m_commandPool;
    allocateInfo.level = vk::CommandBufferLevel::ePrimary;
    allocateInfo.commandBufferCount = commandBuffers.size();

    if (device().allocateCommandBuffers(&allocateInfo, commandBuffers.data()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.command-buffers") << "Failed to create command buffers." << std::endl;
    }
}

void RenderEngine::Impl::createSemaphores()
{
    logger.info("magma.vulkan.render-engine") << "Creating semaphores." << std::endl;

    vk::SemaphoreCreateInfo semaphoreInfo;

    if (device().createSemaphore(&semaphoreInfo, nullptr, m_renderFinishedSemaphore.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine") << "Failed to create semaphores." << std::endl;
    }
}

void RenderEngine::Impl::initRenderScenes()
{
    if (m_renderScenes.size() == 0u) return;

    logger.info("magma.vulkan.render-engine") << "Initializing render scenes." << std::endl;
    logger.log().tab(1);

    for (auto& renderScene : m_renderScenes) {
        renderScene->init();
    }

    logger.log().tab(-1);
}

void RenderEngine::Impl::initVulkan()
{
    logger.info("magma.vulkan.render-engine") << "Initializing vulkan." << std::endl;
    logger.log().tab(1);

    m_instanceHolder.init(true);

    logger.log().tab(-1);
}

void RenderEngine::Impl::initVulkanDevice(vk::SurfaceKHR surface)
{
    logger.info("magma.vulkan.render-engine") << "Initializing vulkan device." << std::endl;
    logger.log().tab(1);

    m_deviceHolder.init(instance(), surface);

    createCommandPool(surface);
    createDummyTextures();
    createSemaphores();

    initRenderScenes();

    logger.log().tab(-1);
}
