#include "./WindowImpl.hpp"

#include <algorithm>
#include <cmath>
#include <lava/crater/Event.hpp>

#include "./Unix/WindowImplX11.hpp"
typedef lava::priv::WindowImplX11 WindowImplType;

using namespace lava::priv;

WindowImpl* WindowImpl::create(VideoMode mode, const String& title, uint32_t style)
{
    return new WindowImplType(mode, title, style);
}

WindowImpl::WindowImpl()
{
}

WindowImpl::~WindowImpl()
{
}

bool WindowImpl::popEvent(Event& event, bool block)
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

void WindowImpl::pushEvent(const Event& event)
{
    m_events.push(event);
}
