#pragma once

#include <cstdint>

// Forward declarations
struct xcb_connection_t;
struct wl_display;
struct wl_surface;

namespace lava {
    struct WsHandle {
        union {
            // XCB handle
            struct {
                xcb_connection_t* connection;
                uint32_t window;
            } xcb;

            // Wayland handle
            struct {
                wl_display* display;
                wl_surface* surface;
            } wayland;

            // Windows' DWM handle
            struct {
                void* hwnd;
                void* hinstance;
            } dwm;
        };
    };
}
