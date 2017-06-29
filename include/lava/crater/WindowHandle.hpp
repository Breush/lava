#pragma once

#include <cstdint>

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

// Forward declaration
struct xcb_connection_t;

namespace lava {

    // XCB handle
    typedef struct {
        xcb_connection_t* connection;
        uint32_t window;
    } WindowHandle;

} // namespace lava
