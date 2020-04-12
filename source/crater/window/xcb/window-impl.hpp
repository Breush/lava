#pragma once

#include "../../interfaces/window-impl.hpp"
#include <lava/crater/window.hpp>

#include <glm/vec2.hpp>
#include <xcb/xcb.h>
#include <xkbcommon/xkbcommon.h>

namespace lava::crater {
    /**
     * XCB-based lava::Window.
     */
    class Window::Impl final : public IWindowImpl {
    public:
        Impl(VideoMode mode, const std::string& title);
        ~Impl();

        // IWindowImpl
        WsHandle handle() const final { return {m_connection, m_window}; }

        bool fullscreen() const final { return m_fullscreen; }
        void fullscreen(bool fullscreen) final;
        bool mouseHidden() const final { return m_mouseHidden; }
        void mouseHidden(bool mouseHidden) final;
        bool mouseKeptCentered() const final { return m_mouseKeptCentered; }
        void mouseKeptCentered(bool mouseKeptCentered) final;

    protected:
        // IWindowImpl
        virtual void processEvents() final;

        void initXcbConnection();
        void setupWindow(VideoMode mode, const std::string& title);
        void setupXkb();
        void processEvent(xcb_generic_event_t& windowEvent);

        void mouseMoveIgnored(bool ignored) const;

    private:
        bool m_fullscreen = false;
        bool m_mouseHidden = false;
        bool m_mouseKeptCentered = false;

        xcb_window_t m_window = -1u;
        xcb_connection_t* m_connection = nullptr;
        xcb_screen_t* m_screen = nullptr;
        xcb_intern_atom_reply_t* m_hintsReply = nullptr;
        xcb_intern_atom_reply_t* m_protocolsReply = nullptr;
        xcb_intern_atom_reply_t* m_deleteWindowReply = nullptr;
        xkb_context* m_xkbContext = nullptr;
        xkb_keymap* m_xkbKeymap = nullptr;
        xkb_state* m_xkbState = nullptr;

        Extent2d m_extent;
        Extent2d m_extentBeforeFullscreen;
        WsEvent::MouseMoveData m_mousePosition;
        bool m_mousePositionToReset = true;

        std::vector<uint32_t> m_ignoredSequences;

        // Mouse hidden mode
        xcb_cursor_t m_emptyCursor = XCB_NONE;

        // Keep cursor centered mode
        bool m_mouseCurrentlyCentered = false;
        WsEvent::MouseMoveData m_mouseMoveAccumulator; // Used in centerCursor @fixme Rename infinite mode
    };
}
