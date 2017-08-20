#pragma once

#include <glm/glm.hpp>

namespace lava::magma::vulkan {
    /// Camera UBO used by GBuffer stage.
    struct CameraUbo {
        glm::mat4 view;
        glm::mat4 projection;
    };

    /// Material UBO used by GBuffer stage.
    struct MaterialUbo {
        float roughnessFactor;
        float metallicFactor;
    };

    /// Mesh UBO used by GBuffer stage.
    struct MeshUbo {
        glm::mat4 transform;
    };
}
