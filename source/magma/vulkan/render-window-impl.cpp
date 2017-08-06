#include "./render-window-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <lava/magma/render-engine.hpp>

#include "./render-engine-impl.hpp"

using namespace lava::magma;
using namespace lava::crater;
using namespace lava::chamber;

RenderWindow::Impl::Impl(RenderEngine& engine, VideoMode mode, const std::string& title)
    : m_engine(engine.impl())
    , m_surface(m_engine.instance().vk())
    , m_swapchainHolder(m_engine)
    , m_renderTargetData({m_swapchainHolder, m_surface})
    , m_window(mode, title)
    , m_windowExtent{mode.width, mode.height}
{
    initSurface();
}

//----- IRenderTarget

void RenderWindow::Impl::init()
{
    initSwapchain();
}

void RenderWindow::Impl::prepare()
{
    auto result = m_swapchainHolder.acquireNextImage();

    if (result == vk::Result::eErrorOutOfDateKHR) {
        // m_engine.recreateSwapchain();
        logger.warning("magma.vulkan.render-window") << "Seems like nobody cares about a out of date swapchain." << std::endl;
        return;
    }
    else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        logger.error("magma.vulkan.render-window") << "Failed to acquire swapchain image." << std::endl;
    }
}

void RenderWindow::Impl::draw(IRenderTarget::UserData data) const
{
    const auto* drawData = reinterpret_cast<const InDataRenderTargetDraw*>(data);
    const uint32_t imageIndex = m_swapchainHolder.currentIndex();

    // Submitting the image back to the swapchain
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &drawData->renderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchainHolder.swapchain();
    presentInfo.pImageIndices = &imageIndex;

    m_engine.presentQueue().presentKHR(presentInfo);
}

void RenderWindow::Impl::refresh()
{
    // m_engine.recreateSwapchain();
}

//----- RenderWindow

bool RenderWindow::Impl::pollEvent(Event& event)
{
    return m_window.pollEvent(event);
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
    // m_engine.recreateSwapchain();
}

bool RenderWindow::Impl::opened() const
{
    return m_window.opened();
}

//----- Internal

void RenderWindow::Impl::initSurface()
{
    // @todo This is platform-specific! Have it work well with any platform
    vk::XcbSurfaceCreateInfoKHR createInfo;
    createInfo.connection = windowHandle().connection;
    createInfo.window = windowHandle().window;

    // @cleanup HPP
    const auto& vk_instance = m_engine.instance().vk();

    if (vk_instance.createXcbSurfaceKHR(&createInfo, nullptr, m_surface.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.surface") << "Unable to create surface for platform." << std::endl;
    }
}

void RenderWindow::Impl::initSwapchain()
{
    m_swapchainHolder.init(m_surface, m_windowExtent);
}
