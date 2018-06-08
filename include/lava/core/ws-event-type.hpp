#pragma once

#include <lava/core/macros.hpp>

$enum_class(lava, WsEventType,
            WindowClosed,        // Window is being closed
            WindowResized,       // Window is being resized
            MouseButtonPressed,  // Mouse button has been pressed
            MouseButtonReleased, // Mouse button has been released
            MouseWheelScrolled,  // Mouse wheel has been scrolled
            MouseMoved,          // Mouse moved
            KeyPressed,          // Keyboard key has been pressed
            KeyReleased,         // Keyboard key has been released
);
