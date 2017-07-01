#pragma once

#include <cstdint>
#include <lava/crater/input/button.hpp>
#include <lava/crater/input/key.hpp>

namespace lava::crater {
    /**
     * An window or input event.
     */
    struct Event {
    public:
        enum Type {
            WindowClosed,
            WindowResized,
            MouseButtonPressed,
            MouseButtonReleased,
            MouseScrolled,
            MouseMoved,
            KeyPressed,
            KeyReleased,
        };

        struct Size {
            uint16_t width;
            uint16_t height;
        };

        struct MouseButton {
            int16_t x;
            int16_t y;
            input::Button which;
        };

        struct MouseMove {
            int16_t x;
            int16_t y;
        };

        struct MouseScroll {
            int16_t x;
            int16_t y;
            int16_t delta;
        };

        struct Key {
            input::Key which;
        };

        // -----

        Type type;
        union {
            Size size;
            MouseButton mouseButton;
            MouseMove mouseMove;
            MouseScroll mouseScroll;
            Key key;
        };
    };
}
