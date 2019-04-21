#pragma once

#include <lava/core/vr-button.hpp>
#include <lava/core/vr-device-type.hpp>
#include <lava/core/vr-event-type.hpp>

namespace lava {
    /**
     * Virtual Reality event.
     */
    struct VrEvent {
    public:
        struct ButtonData {
            VrDeviceType hand; // Which controller (left or right).
            VrButton which;    // Which button is pressed or released.
        };

        // -----

        VrEventType type;
        union {
            ButtonData button;
        };
    };
}
