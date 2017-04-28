////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2017 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <lava/crater/Window.hpp>
#include "./GlContext.hpp"
#include "./WindowImpl.hpp"
#include <lava/chamber/Err.hpp>


namespace
{
    const lava::Window* fullscreenWindow = NULL;
}


namespace lava
{
////////////////////////////////////////////////////////////
Window::Window() :
m_impl          (NULL),
m_context       (NULL),
m_size          (0, 0)
{

}


////////////////////////////////////////////////////////////
Window::Window(VideoMode mode, const String& title, uint32_t style, const ContextSettings& settings) :
m_impl          (NULL),
m_context       (NULL),
m_size          (0, 0)
{
    create(mode, title, style, settings);
}


////////////////////////////////////////////////////////////
Window::~Window()
{
    close();
}


////////////////////////////////////////////////////////////
void Window::create(VideoMode mode, const String& title, uint32_t style, const ContextSettings& settings)
{
    // Destroy the previous window implementation
    close();

    // Fullscreen style requires some tests
    if (style & Style::Fullscreen)
    {
        // Make sure there's not already a fullscreen window (only one is allowed)
        if (fullscreenWindow)
        {
            err() << "Creating two fullscreen windows is not allowed, switching to windowed mode" << std::endl;
            style &= ~Style::Fullscreen;
        }
        else
        {
            // Make sure that the chosen video mode is compatible
            if (!mode.isValid())
            {
                err() << "The requested video mode is not available, switching to a valid mode" << std::endl;
                mode = VideoMode::getFullscreenModes()[0];
            }

            // Update the fullscreen window
            fullscreenWindow = this;
        }
    }

    // Check validity of style according to the underlying platform
    if ((style & Style::Close) || (style & Style::Resize))
        style |= Style::Titlebar;

    // Recreate the window implementation
    m_impl = priv::WindowImpl::create(mode, title, style, settings);

    // Recreate the context
    // TODO
    // m_context = priv::GlContext::create(settings, m_impl, mode.bitsPerPixel);

    // Perform common initializations
    initialize();
}


////////////////////////////////////////////////////////////
void Window::close()
{
    // Delete the context
    delete m_context;
    m_context = NULL;

    // Delete the window implementation
    delete m_impl;
    m_impl = NULL;

    // Update the fullscreen window
    if (this == fullscreenWindow)
        fullscreenWindow = NULL;
}


////////////////////////////////////////////////////////////
bool Window::isOpen() const
{
    return m_impl != NULL;
}


////////////////////////////////////////////////////////////
const ContextSettings& Window::getSettings() const
{
    static const ContextSettings empty(0, 0, 0);

    return m_context ? m_context->getSettings() : empty;
}


////////////////////////////////////////////////////////////
bool Window::pollEvent(Event& event)
{
    if (m_impl && m_impl->popEvent(event, false))
    {
        return filterEvent(event);
    }
    else
    {
        return false;
    }
}


////////////////////////////////////////////////////////////
bool Window::waitEvent(Event& event)
{
    if (m_impl && m_impl->popEvent(event, true))
    {
        return filterEvent(event);
    }
    else
    {
        return false;
    }
}


////////////////////////////////////////////////////////////
Vector2i Window::getPosition() const
{
    return m_impl ? m_impl->getPosition() : Vector2i();
}


////////////////////////////////////////////////////////////
void Window::setPosition(const Vector2i& position)
{
    if (m_impl)
        m_impl->setPosition(position);
}


////////////////////////////////////////////////////////////
Vector2u Window::getSize() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
void Window::setSize(const Vector2u& size)
{
    if (m_impl)
    {
        m_impl->setSize(size);

        // Cache the new size
        m_size.x = size.x;
        m_size.y = size.y;

        // Notify the derived class
        onResize();
    }
}


////////////////////////////////////////////////////////////
void Window::setTitle(const String& title)
{
    if (m_impl)
        m_impl->setTitle(title);
}


////////////////////////////////////////////////////////////
void Window::setIcon(unsigned int width, unsigned int height, const uint8_t* pixels)
{
    if (m_impl)
        m_impl->setIcon(width, height, pixels);
}


////////////////////////////////////////////////////////////
void Window::setVisible(bool visible)
{
    if (m_impl)
        m_impl->setVisible(visible);
}


////////////////////////////////////////////////////////////
void Window::setVerticalSyncEnabled(bool enabled)
{
    if (setActive())
        m_context->setVerticalSyncEnabled(enabled);
}


////////////////////////////////////////////////////////////
void Window::setMouseCursorVisible(bool visible)
{
    if (m_impl)
        m_impl->setMouseCursorVisible(visible);
}


////////////////////////////////////////////////////////////
void Window::setMouseCursorGrabbed(bool grabbed)
{
    if (m_impl)
        m_impl->setMouseCursorGrabbed(grabbed);
}


////////////////////////////////////////////////////////////
void Window::setKeyRepeatEnabled(bool enabled)
{
    if (m_impl)
        m_impl->setKeyRepeatEnabled(enabled);
}


////////////////////////////////////////////////////////////
void Window::setFramerateLimit(unsigned int limit)
{
}


////////////////////////////////////////////////////////////
void Window::setJoystickThreshold(float threshold)
{
    if (m_impl)
        m_impl->setJoystickThreshold(threshold);
}


////////////////////////////////////////////////////////////
bool Window::setActive(bool active) const
{
    if (m_context)
    {
        if (m_context->setActive(active))
        {
            return true;
        }
        else
        {
            err() << "Failed to activate the window's context" << std::endl;
            return false;
        }
    }
    else
    {
        return false;
    }
}


////////////////////////////////////////////////////////////
void Window::requestFocus()
{
    if (m_impl)
        m_impl->requestFocus();
}


////////////////////////////////////////////////////////////
bool Window::hasFocus() const
{
    return m_impl && m_impl->hasFocus();
}


////////////////////////////////////////////////////////////

void Window::display()
{
    // Display the backbuffer on screen
    if (setActive())
        m_context->display();
}


////////////////////////////////////////////////////////////
WindowHandle Window::getSystemHandle() const
{
    return m_impl->getSystemHandle();
}


////////////////////////////////////////////////////////////
void Window::onCreate()
{
    // Nothing by default
}


////////////////////////////////////////////////////////////
void Window::onResize()
{
    // Nothing by default
}


////////////////////////////////////////////////////////////
bool Window::filterEvent(const Event& event)
{
    // Notify resize events to the derived class
    if (event.type == Event::Resized)
    {
        // Cache the new size
        m_size.x = event.size.width;
        m_size.y = event.size.height;

        // Notify the derived class
        onResize();
    }

    return true;
}


////////////////////////////////////////////////////////////
void Window::initialize()
{
    // Setup default behaviors (to get a consistent behavior across different implementations)
    setVisible(true);
    setMouseCursorVisible(true);
    setVerticalSyncEnabled(false);
    setKeyRepeatEnabled(true);
    setFramerateLimit(0);

    // Get and cache the initial size of the window
    m_size = m_impl->getSize();

    // Activate the window
    setActive();

    // Notify the derived class
    onCreate();
}

} // namespace lava
