#pragma once

#include <lava/crater/event.hpp>
#include <lava/crater/video-mode.hpp>
#include <lava/crater/window-handle.hpp>
#include <queue>
#include <string>

namespace lava::crater {
    /**
     * Will be inherited by platform-specific implementations.
     */
    class IWindowImpl {
    public:
        IWindowImpl() = default;
        IWindowImpl(VideoMode videoMode);
        virtual ~IWindowImpl() = default;

        bool popEvent(Event& event);

        virtual WindowHandle windowHandle() const = 0;
        inline VideoMode videoMode() const { return m_videoMode; }

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
