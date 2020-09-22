#pragma once

#include <functional>

namespace lava::sill {
    class IMesh;
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

    std::function<uint32_t(IMesh&)> sphereMeshMaker(uint32_t tessellation, float diameter,
                                                    SphereMeshOptions options = SphereMeshOptions());
}
