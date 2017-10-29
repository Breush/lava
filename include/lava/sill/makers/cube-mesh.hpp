#pragma once

#include <functional>
#include <lava/core/extent.hpp>

namespace lava::sill {
    class MeshComponent;
}

namespace lava::sill {
    enum class CubeSiding {
        OUT,
        IN,
    };

    enum class CubeCoordinatesSystem {
        UNKNOWN,
        BOX_2x3,
    };
}

namespace lava::sill::makers {
    struct CubeMeshOptions {
        CubeSiding siding = CubeSiding::OUT;
        CubeCoordinatesSystem coordinatesSystem = CubeCoordinatesSystem::UNKNOWN;
    };

    std::function<void(MeshComponent& meshComponent)> cubeMeshMaker(float sideLength,
                                                                    CubeMeshOptions options = CubeMeshOptions());
}
