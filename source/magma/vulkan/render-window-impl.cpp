#include "./render-window-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <lava/magma/render-engine.hpp>

#include "./render-engine-impl.hpp"
#include "./swapchain.hpp"

using namespace lava::magma;
using namespace lava::crater;
using namespace lava::chamber;

RenderWindow::Impl::Impl(VideoMode mode, const std::string& title)
    : m_window(mode, title)
    , m_windowExtent({mode.width, mode.height})
{
}

//----- IRenderTarget

void RenderWindow::Impl::init(RenderEngine& engine)
{
    m_engine = &engine.impl();

    m_swapchain = &m_engine->swapchain(); // @todo Create one here instead

    // @todo Add required extensions etc.

    m_engine->m_windowHandle = windowHandle();
    m_engine->m_windowExtent = m_windowExtent;

    m_engine->initVulkan(); // @fixme Wait what?
}

void RenderWindow::Impl::draw() const
{
    if (!m_engine) {
        logger.warning("magma.render-window") << "Attempt to draw without an engine binded." << std::endl;
        return;
    }
}

void RenderWindow::Impl::refresh()
{
    m_engine->recreateSwapchain();
}

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
    m_windowExtent = {mode.width, mode.height};
    m_engine->recreateSwapchain();
}

bool RenderWindow::Impl::opened() const
{
    return m_window.opened();
}
