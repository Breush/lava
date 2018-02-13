#include "./window-impl.hpp"

using namespace lava::crater;

IWindowImpl::IWindowImpl(const VideoMode& videoMode)
    : m_videoMode(videoMode)
{
}

std::optional<lava::WsEvent> IWindowImpl::IWindowImpl::popEvent()
{
    if (m_events.empty()) {
        processEvents();
    }

    if (m_events.empty()) {
        return std::nullopt;
    }

    // Get first event in queue
    auto event = m_events.front();
    m_events.pop();
    return event;
}

void IWindowImpl::pushEvent(const WsEvent& event)
{
    m_events.push(event);
}
