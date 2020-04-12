#pragma once

#include <cstdint>

#include <lava/core/key.hpp>
#include <lava/core/mouse-button.hpp>
#include <lava/core/mouse-wheel.hpp>
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
            int16_t dx;
            int16_t dy;
        };

        struct MouseWheelData {
            int16_t x;
            int16_t y;
            MouseWheel which;

            // Standard delta is 1.f, but hight precision mice (ou touchpads)
            // might be non-integers.
            float delta;
        };

        struct KeyData {
            Key which;
            wchar_t code;
        };

        // -----

        WsEventType type;
        union {
            WindowSizeData windowSize;
            MouseButtonData mouseButton;
            MouseMoveData mouseMove;
            MouseWheelData mouseWheel;
            KeyData key;
        };
    };
}
