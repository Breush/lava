#pragma once

#include <functional>

namespace lava::sill {
    class MeshComponent;
}

namespace lava::sill {
    enum class SphereSiding {
        OUT,
        IN,
    };

    enum class SphereCoordinatesSystem {
        UNKNOWN,
        PANORAMA_SPHERICAL,
    };
}

namespace lava::sill::makers {
    struct SphereMeshOptions {
        SphereSiding siding = SphereSiding::OUT;
        SphereCoordinatesSystem coordinatesSystem = SphereCoordinatesSystem::UNKNOWN;
    };

    std::function<void(MeshComponent& meshComponent)> sphereMeshMaker(uint32_t tessellation, float diameter,
                                                                      SphereMeshOptions options = SphereMeshOptions());
}
