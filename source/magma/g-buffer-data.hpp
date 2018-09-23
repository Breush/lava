#pragma once

namespace lava::magma {
    constexpr const auto G_BUFFER_DATA_SIZE = 12u;

    /// Redefinition of corresponding structure in GLSL.
    struct GBufferData {
        uint32_t data[G_BUFFER_DATA_SIZE];
    };
}
