#include <lava/crater/window.hpp>

#include <lava/chamber/macros.hpp>

// @todo Should be #defined by platforms
#include "./xcb/window-impl.hpp"

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

$pimpl_method_const(Window, WindowHandle, windowHandle);
$pimpl_method_const(Window, VideoMode, videoMode);

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
