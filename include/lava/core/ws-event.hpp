#pragma once

#include <cstdint>
#include <lava/core/key.hpp>
#include <lava/core/mouse-button.hpp>

namespace lava {
    /**
     * Windowing system event.
     */
    struct WsEvent {
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

        struct WindowSizeData {
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
            WindowSizeData windowSize;
            MouseButtonData mouseButton;
            MouseMoveData mouseMove;
            MouseScrollData mouseScroll;
            KeyData key;
        };
    };
}
