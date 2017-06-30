#include "./window-impl.hpp"

#include <lava/crater/event.hpp>

// @todo Should be #defined by platforms
#include "./xcb/window-impl.hpp"
using WindowImpl = lava::WindowImplXcb;

using namespace lava;

Window::Impl* Window::Impl::create(VideoMode mode, const std::string& title)
{
    return new WindowImpl(mode, title);
}

Window::Impl::Impl(VideoMode mode)
    : m_videoMode(mode)
{
}

Window::Impl::~Impl()
{
}

bool Window::Impl::popEvent(Event& event, bool block)
{
    // If the event queue is empty, let's first check if new events are available from the OS
    if (m_events.empty()) {
        // Get events from the system
        processEvents();

        // In blocking mode, we must process events until one is triggered
        if (block) {
            // Here we use a manual wait loop instead of the optimized
            // wait-event provided by the OS, so that we don't skip joystick
            // events (which require polling)
            while (m_events.empty()) {
                // sleep(milliseconds(10));
                processEvents();
            }
        }
    }

    // Pop the first event of the queue, if it is not empty
    if (!m_events.empty()) {
        event = m_events.front();
        m_events.pop();

        return true;
    }

    return false;
}

void Window::Impl::pushEvent(const Event& event)
{
    m_events.push(event);
}
