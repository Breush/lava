#pragma once

#include <functional>
#include <lava/core/extent.hpp>

namespace lava::sill {
    class MeshComponent;
}

namespace lava::sill {
    enum class BoxOrigin {
        Center,
        Bottom,
    };

    enum class BoxSiding {
        Out,
        In,
    };

    enum class BoxCoordinatesSystem {
        Unknown,
        Box2x3,
    };
}

namespace lava::sill::makers {
    struct BoxMeshOptions {
        BoxOrigin origin = BoxOrigin::Center;
        BoxSiding siding = BoxSiding::Out;
        BoxCoordinatesSystem coordinatesSystem = BoxCoordinatesSystem::Unknown;
        glm::vec3 offset = {0.f, 0.f, 0.f};
    };

    std::function<void(MeshComponent&)> boxMeshMaker(float sidesLength, BoxMeshOptions options = BoxMeshOptions());
    std::function<void(MeshComponent&)> boxMeshMaker(const glm::vec3& extent, BoxMeshOptions options = BoxMeshOptions());
}
