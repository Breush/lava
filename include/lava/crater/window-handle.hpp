#pragma once

#include <cstdint>

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

// Forward declarations
struct xcb_connection_t;

namespace lava::crater {
    struct WindowHandle {
        union {
            // XCB handle
            struct {
                xcb_connection_t* connection;
                uint32_t window;
            } xcb;

            // Windows' DWM handle
            struct {
                void* hwnd;
                void* hinstance;
            } dwm;
        };
    };
}
