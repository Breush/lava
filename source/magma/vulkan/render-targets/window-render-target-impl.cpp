#include "./window-render-target-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <lava/magma/render-engine.hpp>

#include "../render-engine-impl.hpp"

using namespace lava::magma;
using namespace lava::crater;
using namespace lava::chamber;

WindowRenderTarget::Impl::Impl(RenderEngine& engine, WsHandle handle, const Extent2d& extent)
    : m_engine(engine.impl())
    , m_handle(handle)
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
}

bool WindowRenderTarget::Impl::prepare()
{
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

void WindowRenderTarget::Impl::draw(vk::Semaphore renderFinishedSemaphore) const
{
    const uint32_t imageIndex = m_swapchainHolder.currentIndex();

    // Submitting the image back to the swapchain
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
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

//----- WindowRenderTarget

void WindowRenderTarget::Impl::extent(const Extent2d& extent)
{
    m_windowExtent.width = extent.width;
    m_windowExtent.height = extent.height;

    recreateSwapchain();
}

//----- Internal

void WindowRenderTarget::Impl::initFence()
{
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    auto result = m_engine.device().createFence(&fenceInfo, nullptr, m_fence.replace());
    if (result != vk::Result::eSuccess) {
        logger.error("magma.vulkan.window-render-target") << "Unable to create fence." << std::endl;
    }
}

void WindowRenderTarget::Impl::initSurface()
{
    vk::Result result;

    const auto wsHandle = handle();

#if defined(VK_USE_PLATFORM_XCB_KHR)
    vk::XcbSurfaceCreateInfoKHR createInfo;
    createInfo.connection = wsHandle.xcb.connection;
    createInfo.window = wsHandle.xcb.window;

    result = m_engine.instance().createXcbSurfaceKHR(&createInfo, nullptr, m_surface.replace());
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    vk::WaylandSurfaceCreateInfoKHR createInfo;
    // @fixme Implement Wayland support

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
    m_swapchainHolder.init(m_surface, m_windowExtent);
}

void WindowRenderTarget::Impl::recreateSwapchain()
{
    m_swapchainHolder.recreate(m_surface, m_windowExtent);

    m_engine.updateRenderTarget(m_id);
}
