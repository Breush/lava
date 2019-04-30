#pragma once

#include <functional>

namespace lava::sill {
    class MeshComponent;
}

namespace lava::sill {
    enum class SphereSiding {
        Out,
        In,
    };

    enum class SphereCoordinatesSystem {
        Unknown,
        PanoramaSpherical,
    };
}

namespace lava::sill::makers {
    struct SphereMeshOptions {
        SphereSiding siding = SphereSiding::Out;
        SphereCoordinatesSystem coordinatesSystem = SphereCoordinatesSystem::Unknown;
    };

    std::function<void(MeshComponent&)> sphereMeshMaker(uint32_t tessellation, float diameter,
                                                        SphereMeshOptions options = SphereMeshOptions());
}
