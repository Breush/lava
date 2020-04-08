#pragma once

namespace lava::sill {
    enum class PickPrecision {
        BoundingSphere, //< Stop whenever a bounding sphere is hit.
        Mesh,           //< Stop whenever a mesh geometry is hit (way slower).
    };
}
