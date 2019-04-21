#pragma once

#include <lava/core/macros.hpp>

$enum_class(lava, InputAxis,
            Unknown,              // Unknown axis
            MouseX,               // Mouse move along x-axis
            MouseY,               // Mouse move along y-axis
            MouseWheelVertical,   // Vertical mouse wheel scrolled
            MouseWheelHorizontal, // Horizontal mouse wheel scrolled
            VrTrigger,            // Trigger button for VR system is pressed
            VrTouchpadX,          // Touchpad for VR system x-axis
            VrTouchpadY,          // Touchpad for VR system y-axis
);
