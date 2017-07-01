#pragma once

#include "../interfaces/window-impl.hpp"
#include <lava/crater/window.hpp>

#include <deque>
#include <glm/vec2.hpp>
#include <lava/crater/event.hpp>
#include <string>
#include <xcb/xcb.h>

namespace lava {
    /**
     * XCB-based lava::Window.
     */
    class Window::Impl final : public IWindowImpl {
    public:
        Impl(VideoMode mode, const std::string& title);

        // IWindowImpl
        WindowHandle windowHandle() const override final;

    protected:
        // IWindowImpl
        virtual void processEvents() override final;

        void initXcbConnection();
        void setupWindow(VideoMode mode, const std::string& title);
        bool processEvent(xcb_generic_event_t& windowEvent);

    private:
        uint32_t m_window = -1u;
        xcb_connection_t* m_connection = nullptr;
        xcb_screen_t* m_screen = nullptr;
        xcb_intern_atom_reply_t* m_atomWmDeleteWindow = nullptr;

        glm::tvec2<int16_t> m_previousSize;
    };
}
