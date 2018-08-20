#pragma once

#include "../../interfaces/window-impl.hpp"
#include <lava/crater/window.hpp>

#include <glm/vec2.hpp>
#include <xcb/xcb.h>

namespace lava::crater {
    /**
     * XCB-based lava::Window.
     */
    class Window::Impl final : public IWindowImpl {
    public:
        Impl(VideoMode mode, const std::string& title);
        ~Impl();

        // IWindowImpl
        WsHandle handle() const override final;

        bool fullscreen() const override final { return m_fullscreen; }
        void fullscreen(bool fullscreen) override final;

    protected:
        // IWindowImpl
        virtual void processEvents() override final;

        void initXcbConnection();
        void setupWindow(VideoMode mode, const std::string& title);
        bool processEvent(xcb_generic_event_t& windowEvent);

    private:
        bool m_fullscreen = false;

        xcb_window_t m_window = -1u;
        xcb_connection_t* m_connection = nullptr;
        xcb_screen_t* m_screen = nullptr;
        xcb_intern_atom_reply_t* m_hintsReply = nullptr;
        xcb_intern_atom_reply_t* m_protocolsReply = nullptr;
        xcb_intern_atom_reply_t* m_deleteWindowReply = nullptr;

        Extent2d m_extent;
        Extent2d m_extentBeforeFullscreen;
    };
}
