#pragma once

#include <functional>

namespace lava::sill {
    class MeshComponent;
}

namespace lava::sill::makers {
    struct CylinderMeshOptions {
        bool doubleSided = false;
    };

    std::function<void(MeshComponent&)> cylinderMeshMaker(uint32_t tessellation, float diameter, float length,
                                                          CylinderMeshOptions options = CylinderMeshOptions());
}
