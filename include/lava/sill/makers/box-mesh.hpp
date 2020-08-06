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
        None,
        Box2x3,   // Used for packed skyboxes texture (two rows, three columns)
        FullFace, // Each face has 0,0 -> 1,1 UVs
    };
}

namespace lava::sill::makers {
    struct BoxMeshOptions {
        BoxOrigin origin = BoxOrigin::Center;
        BoxSiding siding = BoxSiding::Out;
        BoxCoordinatesSystem coordinatesSystem = BoxCoordinatesSystem::FullFace;
        glm::vec3 offset = {0.f, 0.f, 0.f};
    };

    std::function<void(MeshComponent&)> boxMeshMaker(float sidesLength, const BoxMeshOptions& options = BoxMeshOptions());
    std::function<void(MeshComponent&)> boxMeshMaker(const glm::vec3& extent, const BoxMeshOptions& options = BoxMeshOptions());
}
