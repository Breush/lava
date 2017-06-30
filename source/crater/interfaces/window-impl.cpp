#include "./window-impl.hpp"

#include <lava/crater/event.hpp>

using namespace lava;

IWindowImpl::IWindowImpl(VideoMode videoMode)
    : m_videoMode(videoMode)
{
}

bool IWindowImpl::IWindowImpl::popEvent(Event& event)
{
    if (m_events.empty()) {
        processEvents();
    }

    if (m_events.empty()) {
        return false;
    }

    // Get first event in queue
    event = m_events.front();
    m_events.pop();
    return true;
}

void IWindowImpl::pushEvent(const Event& event)
{
    m_events.push(event);
}
