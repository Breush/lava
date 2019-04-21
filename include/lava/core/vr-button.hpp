#pragma once

#include <lava/core/macros.hpp>

$enum_class(lava, VrButton,
            Unknown,  // Unknown
            System,   // System button (⧉)
            Menu,     // Application menu button (≡)
            Trigger,  // SteamVR trigger button
            Touchpad, // SteamVR touchpad button
);
