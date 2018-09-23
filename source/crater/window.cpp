#include <lava/crater/window.hpp>

#if defined(LAVA_CRATER_WINDOW_XCB)
#include "./window/xcb/window-impl.hpp"
#elif defined(LAVA_CRATER_WINDOW_WAYLAND)
#include "./window/wayland/window-impl.hpp"
#elif defined(LAVA_CRATER_WINDOW_DWM)
#include "./window/dwm/window-impl.hpp"
#else
#error "[lava.crater.window] No windowing system defined."
#endif

using namespace lava::crater;

Window::Window() {}

Window::Window(VideoMode mode, const std::string& title)
{
    create(mode, title);
}

Window::~Window()
{
    close();
}

$pimpl_attribute_v(Window, lava::WsHandle, handle);
$pimpl_attribute(Window, VideoMode, videoMode);
$pimpl_property_v(Window, bool, fullscreen);

lava::Extent2d Window::extent() const
{
    const auto& mode = videoMode();
    return {mode.width, mode.height};
}

void Window::create(VideoMode mode, const std::string& title)
{
    close();
    m_impl = new Impl(mode, title);
}

void Window::close()
{
    delete m_impl;
    m_impl = nullptr;
}

bool Window::opened() const
{
    return m_impl != nullptr;
}

std::optional<lava::WsEvent> Window::pollEvent()
{
    if (m_impl == nullptr) return std::nullopt;
    return m_impl->popEvent();
}
