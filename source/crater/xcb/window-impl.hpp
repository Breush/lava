#pragma once

#include "../window-impl.hpp"

#include <deque>
#include <glm/vec2.hpp>
#include <lava/crater/event.hpp>
#include <string>
#include <xcb/xcb.h>

namespace lava::priv {

    /// \brief Linux (X11) implementation of WindowImpl
    ///

    class WindowImplX11 : public WindowImpl {
    public:
        /// \brief Create the window implementation
        ///
        /// \param mode  Video mode to use
        /// \param title Title of the window
        /// \param style Window style (resizable, fixed, or fullscren)
        /// \param settings Additional settings for the underlying OpenGL context
        WindowImplX11(VideoMode mode, const std::string& title, unsigned long style);

        ~WindowImplX11();

        /// \brief Get the OS-specific handle of the window
        ///
        /// \return Handle of the window
        WindowHandle getSystemHandle() const override final;

    protected:
        /// \brief Process incoming events from the operating system
        virtual void processEvents() override;

        void initXcbConnection();

        void setupWindow(VideoMode mode);

        /// \brief Set fullscreen video mode
        ///
        /// \param Mode video mode to switch to
        void setVideoMode(const VideoMode& mode);

        /// \brief Reset to desktop video mode
        void resetVideoMode();

        /// \brief Cleanup graphical resources attached to the window
        void cleanup();

        /// \brief Process an incoming event from the window
        ///
        /// \param windowEvent Event which has been received
        ///
        /// \return True if the event was processed, false if it was discarded
        bool processEvent(xcb_generic_event_t& windowEvent);

    private:
        uint32_t m_window = -1u;
        xcb_connection_t* m_connection = nullptr;
        xcb_screen_t* m_screen = nullptr;
        xcb_intern_atom_reply_t* m_atomWmDeleteWindow = nullptr;

        glm::tvec2<int16_t> m_previousSize;
    };
}
