#include <lava/crater/Window.hpp>

#include <lava/chamber/Err.hpp>

#include "./WindowImpl.hpp"

namespace {
    const lava::Window* fullscreenWindow = NULL;
}

namespace lava {
    Window::Window() {}
    Window::Window(VideoMode mode, const String& title, uint32_t style) { create(mode, title, style); }

    Window::~Window() { close(); }

    void Window::create(VideoMode mode, const String& title, uint32_t style)
    {
        // Destroy the previous window implementation
        close();

        // Fullscreen style requires some tests
        if (style & Style::Fullscreen) {
            // Make sure there's not already a fullscreen window (only one is allowed)
            if (fullscreenWindow) {
                err() << "Creating two fullscreen windows is not allowed, switching to windowed mode" << std::endl;
                style &= ~Style::Fullscreen;
            }
            else {
                // Make sure that the chosen video mode is compatible
                if (!mode.isValid()) {
                    err() << "The requested video mode is not available, switching to a valid mode" << std::endl;
                    mode = VideoMode::getFullscreenModes()[0];
                }

                // Update the fullscreen window
                fullscreenWindow = this;
            }
        }

        // Check validity of style according to the underlying platform
        if ((style & Style::Close) || (style & Style::Resize)) style |= Style::Titlebar;

        // Recreate the window implementation
        m_impl = priv::WindowImpl::create(mode, title, style);
    }

    void Window::close()
    {
        // Delete the window implementation
        delete m_impl;
        m_impl = NULL;

        // Update the fullscreen window
        if (this == fullscreenWindow) fullscreenWindow = NULL;
    }

    bool Window::isOpen() const { return m_impl != NULL; }

    bool Window::pollEvent(Event& event)
    {
        if (m_impl && m_impl->popEvent(event, false)) {
            return true;
        }

        return false;
    }

    bool Window::waitEvent(Event& event)
    {
        if (m_impl && m_impl->popEvent(event, true)) {
            return true;
        }

        return false;
    }

    WindowHandle Window::getSystemHandle() const { return m_impl->getSystemHandle(); }

    void Window::onCreate()
    {
        // Nothing by default
    }

    void Window::onResize()
    {
        // Nothing by default
    }
}
