#pragma once

namespace lava {
    struct Viewport {
        float x = 0.f;
        float y = 0.f;
        float width = 1.f;
        float height = 1.f;
        float depth = 0.f; // Deeper viewport are rendered in background.
    };
};
