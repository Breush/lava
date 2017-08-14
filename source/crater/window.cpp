#include <lava/crater/window.hpp>

#include <lava/chamber/macros.hpp>

#if defined(LAVA_CRATER_WINDOW_XCB)
#include "./window/xcb/window-impl.hpp"
#elif defined(LAVA_CRATER_WINDOW_DWM)
#include "./window/dwm/window-impl.hpp"
#else
#error "[lava.crater.window] No windowing system defined."
#endif

using namespace lava::crater;

Window::Window()
{
}

Window::Window(VideoMode mode, const std::string& title)
{
    create(mode, title);
}

Window::~Window()
{
    close();
}

$pimpl_attribute_v(Window, WindowHandle, windowHandle);
$pimpl_attribute(Window, VideoMode, videoMode);

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

bool Window::pollEvent(Event& event)
{
    if (m_impl == nullptr) return false;
    return m_impl->popEvent(event);
}
