#pragma once

namespace lava {
    enum class InterpolationEase {
        None, // t
        In,   // t²
        Out,  // 1 - (1 - t)²
    };
}
