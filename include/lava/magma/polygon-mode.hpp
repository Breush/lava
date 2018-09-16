#pragma once

namespace lava::magma {
    enum class PolygonMode {
        Unknown,
        Fill, // Default, triangles are filled.
        Line, // Wireframe rendering.
    };
}
