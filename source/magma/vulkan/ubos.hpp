#pragma once

#include <glm/glm.hpp>

namespace lava::magma::vulkan {
    /// Camera UBO used by GBuffer stage.
    struct CameraUbo {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 wPosition;
    };

    /// Material UBO used by GBuffer stage.
    struct MaterialUbo {
        union {
            uint32_t id;
            glm::vec4 __padding_id;
        };
        uint32_t data[8];

        MaterialUbo() {}
    };

    /// Mesh UBO used by GBuffer stage.
    struct MeshUbo {
        glm::mat4 transform;
    };
}
