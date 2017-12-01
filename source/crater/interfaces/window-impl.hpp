#pragma once

#include <lava/crater/event.hpp>
#include <lava/crater/video-mode.hpp>
#include <lava/crater/window-handle.hpp>

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

        std::optional<Event> popEvent();

        virtual WindowHandle windowHandle() const = 0;
        const VideoMode& videoMode() const { return m_videoMode; }

    protected:
        void pushEvent(const Event& event);

        /**
         * Add all events from the system.
         */
        virtual void processEvents() = 0;

    private:
        std::queue<Event> m_events;
        VideoMode m_videoMode;
    };
}
