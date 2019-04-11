#include "./window-render-target-impl.hpp"

#include <lava/magma/render-engine.hpp>

#include "../render-engine-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

WindowRenderTarget::Impl::Impl(RenderEngine& engine, WsHandle handle, const Extent2d& extent)
    : m_engine(engine.impl())
    , m_handle(handle)
    , m_presentStage(m_engine)
    , m_renderFinishedSemaphore(m_engine.device())
    , m_fence(m_engine.device())
    , m_surface(m_engine.instance())
    , m_swapchainHolder(m_engine)
{
    m_windowExtent.width = extent.width;
    m_windowExtent.height = extent.height;

    initSurface();
}

//----- IRenderTarget

void WindowRenderTarget::Impl::init(uint32_t id)
{
    m_id = id;

    initFence();
    initSwapchain();
    initPresentStage();
    createSemaphore();
}

bool WindowRenderTarget::Impl::prepare()
{
    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    static const auto MAX = std::numeric_limits<uint64_t>::max();

    m_engine.device().waitForFences(1u, &m_fence, true, MAX);

    auto result = m_swapchainHolder.acquireNextImage();

    if (result == vk::Result::eErrorOutOfDateKHR) {
        // @note We should not go this pass to often, it would mean that
        // the user chose to not our extent() function when he got
        // a window size event.
        recreateSwapchain();
        return false;
    }
    else if (result == vk::Result::eSuboptimalKHR) {
        logger.warning("magma.vulkan.window-render-target") << "Suboptimal swapchain." << std::endl;
        return false;
    }
    else if (result != vk::Result::eSuccess) {
        logger.error("magma.vulkan.window-render-target") << "Failed to acquire swapchain image." << std::endl;
        return false;
    }

    m_engine.device().resetFences(1u, &m_fence);
    return true;
}

void WindowRenderTarget::Impl::render(vk::CommandBuffer commandBuffer)
{
    PROFILE_FUNCTION(PROFILER_COLOR_RENDER);

    m_presentStage.render(commandBuffer);
}

void WindowRenderTarget::Impl::draw(const std::vector<vk::CommandBuffer>& commandBuffers) const
{
    PROFILE_FUNCTION(PROFILER_COLOR_DRAW);

    // Submit it to the queue
    vk::Semaphore waitSemaphores[] = {m_swapchainHolder.imageAvailableSemaphore()};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submitInfo;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderFinishedSemaphore;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    if (m_engine.graphicsQueue().submit(1, &submitInfo, m_fence) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.window-render-target") << "Failed to submit draw command buffer." << std::endl;
    }

    const uint32_t imageIndex = m_swapchainHolder.currentIndex();

    // Submitting the image back to the swapchain
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchainHolder.swapchain();
    presentInfo.pImageIndices = &imageIndex;

    // @note Passing a reference to presentInfo would go into the enhanced
    // version of the vulkan.hpp's presentKHR(). Doing so, it enables
    // asserts or exceptions on eErrorOutOfDateKHR which does not let us
    // just ignore it.
    auto result = m_engine.presentQueue().presentKHR(&presentInfo);

    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR && result != vk::Result::eErrorOutOfDateKHR) {
        logger.error("magma.vulkan.window-render-target") << "Failed to draw: " << vk::to_string(result) << "." << std::endl;
    }
}

uint32_t WindowRenderTarget::Impl::addView(vk::ImageView imageView, vk::ImageLayout imageLayout, vk::Sampler sampler,
                                           Viewport viewport)
{
    // @fixme All these view managements should be a compositor thingy, and not present's business.
    return m_presentStage.addView(imageView, imageLayout, sampler, viewport);
}

void WindowRenderTarget::Impl::removeView(uint32_t viewId)
{
    m_presentStage.removeView(viewId);
}

void WindowRenderTarget::Impl::updateView(uint32_t viewId, vk::ImageView imageView, vk::ImageLayout imageLayout,
                                          vk::Sampler sampler)
{
    m_presentStage.updateView(viewId, imageView, imageLayout, sampler);
}

//----- WindowRenderTarget

void WindowRenderTarget::Impl::extent(const Extent2d& extent)
{
    m_windowExtent.width = extent.width;
    m_windowExtent.height = extent.height;

    recreateSwapchain();
}

//----- Internal

void WindowRenderTarget::Impl::initPresentStage()
{
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    m_presentStage.bindSwapchainHolder(m_swapchainHolder);
    m_presentStage.init();
    m_presentStage.update(m_swapchainHolder.extent());
}

void WindowRenderTarget::Impl::initFence()
{
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    auto result = m_engine.device().createFence(&fenceInfo, nullptr, m_fence.replace());
    if (result != vk::Result::eSuccess) {
        logger.error("magma.vulkan.window-render-target") << "Unable to create fence." << std::endl;
    }
}

void WindowRenderTarget::Impl::initSurface()
{
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    vk::Result result;

    const auto wsHandle = handle();

#if defined(VK_USE_PLATFORM_XCB_KHR)
    vk::XcbSurfaceCreateInfoKHR createInfo;
    createInfo.connection = wsHandle.xcb.connection;
    createInfo.window = wsHandle.xcb.window;

    result = m_engine.instance().createXcbSurfaceKHR(&createInfo, nullptr, m_surface.replace());
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    vk::WaylandSurfaceCreateInfoKHR createInfo;
    createInfo.display = wsHandle.wayland.display;
    createInfo.surface = wsHandle.wayland.surface;

    result = m_engine.instance().createWaylandSurfaceKHR(&createInfo, nullptr, m_surface.replace());
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    vk::Win32SurfaceCreateInfoKHR createInfo;
    createInfo.hwnd = reinterpret_cast<HWND>(wsHandle.dwm.hwnd);
    createInfo.hinstance = reinterpret_cast<HINSTANCE>(wsHandle.dwm.hinstance);

    result = m_engine.instance().createWin32SurfaceKHR(&createInfo, nullptr, m_surface.replace());
#endif

    if (result != vk::Result::eSuccess) {
        logger.error("magma.vulkan.surface") << "Unable to create surface for the platform." << std::endl;
    }
}

void WindowRenderTarget::Impl::initSwapchain()
{
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    m_swapchainHolder.init(m_surface, m_windowExtent);
}

void WindowRenderTarget::Impl::recreateSwapchain()
{
    PROFILE_FUNCTION();

    m_swapchainHolder.recreate(m_surface, m_windowExtent);
    m_presentStage.update(m_swapchainHolder.extent());
}

void WindowRenderTarget::Impl::createSemaphore()
{
    PROFILE_FUNCTION(PROFILER_COLOR_ALLOCATION);

    logger.info("magma.vulkan.render-engine") << "Creating semaphores." << std::endl;

    vk::SemaphoreCreateInfo semaphoreInfo;

    if (m_engine.device().createSemaphore(&semaphoreInfo, nullptr, m_renderFinishedSemaphore.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.render-engine") << "Failed to create semaphores." << std::endl;
    }

    m_engine.deviceHolder().debugObjectName(m_renderFinishedSemaphore, "render-engine.render-finished");
}
