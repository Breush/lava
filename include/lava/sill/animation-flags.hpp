#pragma once

namespace lava::sill {
    /**
     * Used to indicate which component to animate over time.
     */
    enum AnimationFlag : uint32_t {
        None = 0x0000,
        WorldTransform = 0x0001,
    };

    using AnimationFlags = uint32_t;
}
