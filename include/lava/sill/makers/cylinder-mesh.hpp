#pragma once

#include <functional>

namespace lava::sill {
    class MeshComponent;
}

namespace lava::sill::makers {
    struct CylinderMeshOptions {
        bool doubleSided = false;
        glm::mat4 transform = glm::mat4(1.f); // Baked transform of the vertices if any.
        float offset = 0.f; // Vertices default to centered around zero, this allows on offset along the main axis.
    };

    std::function<void(MeshComponent&)> cylinderMeshMaker(uint32_t tessellation, float diameter, float length,
                                                          const CylinderMeshOptions& options = CylinderMeshOptions());
}
