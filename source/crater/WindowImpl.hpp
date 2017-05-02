#pragma once

#include <lava/config.hpp>

#include <lava/chamber/NonCopyable.hpp>
#include <lava/chamber/String.hpp>
#include <lava/chamber/Vector2.hpp>
#include <lava/crater/Event.hpp>
#include <lava/crater/VideoMode.hpp>
#include <lava/crater/WindowHandle.hpp>

#include <queue>
#include <set>

namespace lava {
    class WindowListener;
}

namespace lava::priv {

    /// \brief Abstract base class for OS-specific window implementation
    class WindowImpl : NonCopyable {
    public:
        /// \brief Create a new window depending on the current OS
        ///
        /// \param mode  Video mode to use
        /// \param title Title of the window
        /// \param style Window style
        /// \param settings Additional settings for the underlying OpenGL context
        ///
        /// \return Pointer to the created window (don't forget to delete it)
        static WindowImpl* create(VideoMode mode, const String& title, uint32_t style);

    public:
        WindowImpl(VideoMode mode);
        virtual ~WindowImpl();

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

        virtual WindowHandle getSystemHandle() const = 0;

        inline VideoMode videoMode() const { return m_videoMode; }

    protected:
        WindowImpl();

        /// \brief Push a new event into the event queue
        ///
        /// This function is to be used by derived classes, to
        /// notify the SFML window that a new event was triggered
        /// by the system.
        ///
        /// \param event Event to push
        void pushEvent(const Event& event);

        /// \brief Process incoming events from the operating system
        virtual void processEvents() = 0;

    private:
        std::queue<Event> m_events; ///< Queue of available events

        VideoMode m_videoMode;
    };
}
