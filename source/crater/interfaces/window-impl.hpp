#pragma once

#include <lava/core/ws-event.hpp>
#include <lava/core/ws-handle.hpp>
#include <lava/crater/video-mode.hpp>

#include <optional>
#include <queue>
#include <string>

namespace lava::crater {
    /**
     * Will be inherited by platform-specific implementations.
     */
    class IWindowImpl {
    public:
        IWindowImpl() = default;
        IWindowImpl(const VideoMode& videoMode);
        virtual ~IWindowImpl() = default;

        std::optional<WsEvent> popEvent();

        virtual WsHandle handle() const = 0;
        const VideoMode& videoMode() const { return m_videoMode; }

    protected:
        void pushEvent(const WsEvent& event);

        /**
         * Add all events from the system.
         */
        virtual void processEvents() = 0;

    private:
        std::queue<WsEvent> m_events;
        VideoMode m_videoMode;
    };
}
