#include <lava/crater/window.hpp>

#include <lava/chamber/logger.hpp>

#include "./window-impl.hpp"

using namespace lava;

// @fixme Use pimpl
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

void Window::create(VideoMode mode, const std::string& title)
{
    close();

    m_impl = Window::Impl::create(mode, title);
}

void Window::close()
{
    // Delete the window implementation
    delete m_impl;
    m_impl = nullptr;
}

bool Window::opened() const
{
    return m_impl != nullptr;
}

bool Window::pollEvent(Event& event)
{
    if (m_impl && m_impl->popEvent(event, false)) {
        return true;
    }

    return false;
}

WindowHandle Window::windowHandle() const
{
    return m_impl->windowHandle();
}

VideoMode Window::videoMode() const
{
    return m_impl->videoMode();
}
