#pragma once

#include <cstdint>

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

// Forward declarations
// @todo This should not be visible by the API users
struct xcb_connection_t;

namespace lava::crater {

    typedef struct {
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
    } WindowHandle;

} // namespace lava
