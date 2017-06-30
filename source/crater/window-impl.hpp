#pragma once

#include <lava/crater/window.hpp>

#include <lava/crater/event.hpp>
#include <lava/crater/video-mode.hpp>
#include <lava/crater/window-handle.hpp>
#include <queue>
#include <set>
#include <string>

namespace lava {
    class WindowListener;
}

namespace lava {
    /**
     * Will be inherited by platform-specific implementations.
     */
    class Window::Impl {
    public:
        // @fixme Why a static?
        static Impl* create(VideoMode mode, const std::string& title);

    public:
        Impl();
        Impl(VideoMode mode);
        virtual ~Impl();

        /// \brief Return the next window event available
        ///
        /// If there's no event available, this function calls the
        /// window's internal event processing function.
        /// The \a block parameter controls the behavior of the function
        /// if no event is available: if it is true then the function
        /// doesn't return until a new event is triggered; otherwise it
        /// returns false to indicate that no event is available.
        ///
        /// \param event Event to be returned
        /// \param block Use true to block the thread until an event arrives
        bool popEvent(Event& event, bool block);

        /// \brief Get the OS-specific handle of the window
        ///
        /// \return Handle of the window

        virtual WindowHandle windowHandle() const = 0;

        inline VideoMode videoMode() const { return m_videoMode; }

    protected:
        /// \brief Push a new event into the event queue
        ///
        /// This function is to be used by derived classes, to
        /// notify the window that a new event was triggered
        /// by the system.
        ///
        /// \param event Event to push
        void pushEvent(const Event& event);

        /// \brief Process incoming events from the operating system
        virtual void processEvents() = 0;

    private:
        std::queue<Event> m_events;

        VideoMode m_videoMode;
    };
}
