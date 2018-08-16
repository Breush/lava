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

    initSwapchain();
}

void WindowRenderTarget::Impl::prepare()
{
    auto result = m_swapchainHolder.acquireNextImage();

    if (result == vk::Result::eErrorOutOfDateKHR) {
        logger.warning("magma.vulkan.window-render-target")
            << "Seems like nobody cares about a out of date swapchain." << std::endl;
    }
    else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        logger.error("magma.vulkan.window-render-target") << "Failed to acquire swapchain image." << std::endl;
    }
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

    // @note Somehow, unable to find a better way to prevent
    // an OutOfDateKHRError on Linux during resize...
    try {
        m_engine.presentQueue().presentKHR(presentInfo);
    } catch (vk::OutOfDateKHRError err) {
        const_cast<Impl*>(this)->recreateSwapchain();
    }
}

//----- WindowRenderTarget

void WindowRenderTarget::Impl::extent(const Extent2d& extent)
{
    m_windowExtent.width = extent.width;
    m_windowExtent.height = extent.height;
    recreateSwapchain();

    m_engine.updateRenderTarget(m_id);
}

//----- Internal

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
}
