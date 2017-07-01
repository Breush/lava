#pragma once

#include <cstdint>

namespace lava::crater {
    struct VideoMode {
    public:
        uint16_t width = 0u;
        uint16_t height = 0u;
        uint16_t bitsPerPixel = 0u;

        VideoMode(uint16_t width, uint16_t height, uint16_t bitsPerPixel = 32u);
    };
}
