#pragma once

#include <lava/magma/mesh.hpp>

namespace lava::magma {
    /**
     * This sphere mesh builder is used for bounding sphere debugging.
     * It creates a sphere of radius 1 centered in (0, 0, 0),
     * with a tessellation of 8.
     */
    void buildSphereMesh(Mesh& mesh);
}
