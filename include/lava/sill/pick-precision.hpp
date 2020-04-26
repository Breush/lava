#pragma once

namespace lava::sill {
    enum class PickPrecision {
        BoundingSphere, //< Stop whenever a bounding sphere is hit.
        Collider,       //< Stop whenever a physics geometry is hit (slower, but pretty fast).
        Mesh,           //< Stop whenever a mesh geometry is hit (way slower).
    };
}
