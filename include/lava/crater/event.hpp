#pragma once

#include <cstdint>
#include <lava/crater/key.hpp>
#include <lava/crater/mouse-button.hpp>

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

        struct MouseButtonData {
            int16_t x;
            int16_t y;
            MouseButton which;
        };

        struct MouseMoveData {
            int16_t x;
            int16_t y;
        };

        struct MouseScrollData {
            int16_t x;
            int16_t y;
            int16_t delta;
        };

        struct KeyData {
            Key which;
        };

        // -----

        Type type;
        union {
            Size size;
            MouseButtonData mouseButton;
            MouseMoveData mouseMove;
            MouseScrollData mouseScroll;
            KeyData key;
        };
    };
}
