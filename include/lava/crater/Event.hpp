#pragma once

#include <lava/config.hpp>
#include <lava/crater/keyboard.hpp>
#include <lava/crater/mouse.hpp>

namespace lava {

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
            MouseMoved,
            KeyPressed,
            KeyReleased,
        };

        struct Size {
            uint16_t width;
            uint16_t height;
        };

        struct MouseButton {
            Mouse::Button which;
            int16_t x;
            int16_t y;
        };

        struct MouseMove {
            int16_t x;
            int16_t y;
        };

        struct Key {
            Keyboard::Key which;
        };

        // -----

        Type type;
        union {
            Size size;
            MouseButton mouseButton;
            MouseMove mouseMove;
            Key key;
        };
    };
}
