#pragma once

#include <functional>
#include <lava/core/extent.hpp>

namespace lava::sill {
    class MeshComponent;
}

namespace lava::sill::makers {
    struct PlaneMeshOptions {
        glm::uvec2 tessellation = {2u, 2u}; // How many points per side (for X and Y axis).
        bool doubleSided = false;
    };

    std::function<void(MeshComponent&)> planeMeshMaker(float sidesLength, PlaneMeshOptions options = PlaneMeshOptions());
    std::function<void(MeshComponent&)> planeMeshMaker(const glm::vec2& dimensions,
                                                       PlaneMeshOptions options = PlaneMeshOptions());
}
