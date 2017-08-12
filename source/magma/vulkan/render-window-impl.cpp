#include "./render-window-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <lava/magma/render-engine.hpp>

#include "./render-engine-impl.hpp"

using namespace lava::magma;
using namespace lava::crater;
using namespace lava::chamber;

RenderWindow::Impl::Impl(RenderEngine& engine, VideoMode mode, const std::string& title)
    : m_engine(engine.impl())
    , m_surface(m_engine.instance())
    , m_swapchainHolder(m_engine)
    , m_renderTargetData({m_swapchainHolder, m_surface})
    , m_window(mode, title)
    , m_windowExtent{mode.width, mode.height}
{
    initSurface();
}

//----- IRenderTarget

void RenderWindow::Impl::init(IRenderTarget::UserData data)
{
    const auto& initData = *reinterpret_cast<const InDataRenderTargetInit*>(data);
    m_id = initData.id;

    initSwapchain();
}

void RenderWindow::Impl::prepare()
{
    auto result = m_swapchainHolder.acquireNextImage();

    if (result == vk::Result::eErrorOutOfDateKHR) {
        logger.warning("magma.vulkan.render-window") << "Seems like nobody cares about a out of date swapchain." << std::endl;
    }
    else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        logger.error("magma.vulkan.render-window") << "Failed to acquire swapchain image." << std::endl;
    }
}

void RenderWindow::Impl::draw(IRenderTarget::UserData data) const
{
    const auto& drawData = *reinterpret_cast<const InDataRenderTargetDraw*>(data);
    const uint32_t imageIndex = m_swapchainHolder.currentIndex();

    // Submitting the image back to the swapchain
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &drawData.renderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchainHolder.swapchain();
    presentInfo.pImageIndices = &imageIndex;

    m_engine.presentQueue().presentKHR(presentInfo);
}

//----- RenderWindow

bool RenderWindow::Impl::pollEvent(Event& event)
{
    auto foundEvent = m_window.pollEvent(event);
    if (foundEvent && event.type == crater::Event::WindowResized) {
        // Ignore resize of same size
        if (m_windowExtent.width == event.size.width && m_windowExtent.height == event.size.height) {
            return false;
        }

        // Or update swapchain
        m_windowExtent.width = event.size.width;
        m_windowExtent.height = event.size.height;
        recreateSwapchain();

        m_engine.updateRenderTarget(m_id);
    }
    return foundEvent;
}

void RenderWindow::Impl::close()
{
    m_window.close();
}

WindowHandle RenderWindow::Impl::windowHandle() const
{
    return m_window.windowHandle();
}

VideoMode RenderWindow::Impl::videoMode() const
{
    return m_window.videoMode();
}

void RenderWindow::Impl::videoMode(const VideoMode& mode)
{
    m_windowExtent.width = mode.width;
    m_windowExtent.height = mode.height;
    recreateSwapchain();
}

bool RenderWindow::Impl::opened() const
{
    return m_window.opened();
}

//----- Internal

void RenderWindow::Impl::initSurface()
{
    vk::Result result;

    const auto handle = windowHandle();

#if defined(VK_USE_PLATFORM_XCB_KHR)
    vk::XcbSurfaceCreateInfoKHR createInfo;
    createInfo.connection = handle.xcb.connection;
    createInfo.window = handle.xcb.window;

    result = m_engine.instance().createXcbSurfaceKHR(&createInfo, nullptr, m_surface.replace());
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    vk::Win32SurfaceCreateInfoKHR createInfo;
    createInfo.hwnd = reinterpret_cast<HWND>(handle.dwm.hwnd);
    createInfo.hinstance = reinterpret_cast<HINSTANCE>(handle.dwm.hinstance);

    result = m_engine.instance().createWin32SurfaceKHR(&createInfo, nullptr, m_surface.replace());
#endif

    if (result != vk::Result::eSuccess) {
        logger.error("magma.vulkan.surface") << "Unable to create surface for platform." << std::endl;
    }
}

void RenderWindow::Impl::initSwapchain()
{
    m_swapchainHolder.init(m_surface, m_windowExtent);
}

void RenderWindow::Impl::recreateSwapchain()
{
    m_swapchainHolder.recreate(m_surface, m_windowExtent);
}
