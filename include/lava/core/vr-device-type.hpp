#pragma once

#include <lava/core/macros.hpp>

$enum_class(lava, VrDeviceType,
            Head,        // Head-mounted device (HMD)
            UnknownHand, // Controller, but not identified yet
            LeftHand,    // Left controller (assumed)
            RightHand,   // Right controller (assumed)
);
