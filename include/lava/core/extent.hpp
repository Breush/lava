#pragma once

#include <cstdint>

namespace lava {
    struct Extent2d {
        uint32_t width = 0u;
        uint32_t height = 0u;
    };

    struct FloatExtent2d {
        float width = 0.f;
        float height = 0.f;
    };
};
