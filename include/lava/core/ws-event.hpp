#pragma once

#include <cstdint>

#include <lava/core/key.hpp>
#include <lava/core/mouse-button.hpp>
#include <lava/core/ws-event-type.hpp>

namespace lava {
    /**
     * Windowing system event.
     */
    struct WsEvent {
    public:
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

            // Standard delta is 1.f, but hight precision mice (ou touchpads)
            // might be non-integers.
            float delta;
        };

        struct KeyData {
            Key which;
        };

        // -----

        WsEventType type;
        union {
            WindowSizeData windowSize;
            MouseButtonData mouseButton;
            MouseMoveData mouseMove;
            MouseScrollData mouseScroll;
            KeyData key;
        };
    };
}
