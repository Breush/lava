#pragma once

namespace lava::sill {
    enum class PickPrecision {
        BoundingSphere, //< Stop whenever a bounding sphere is hit.
        BoundingBox,    //< Stop whenever a bounding box is hit (a bit slower).
        Mesh,           //< Stop whenever a mesh geometry is hit (way slower).
    };
}
