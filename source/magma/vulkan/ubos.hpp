#pragma once

#include <glm/glm.hpp>

// @todo What's vulkan?
namespace lava::magma::vulkan {
    /// Camera UBO used by GBuffer stage.
    struct CameraUbo {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 wPosition;
    };

    /// Material UBO used by GBuffer stage.
    struct MaterialUboHeader {
        union {
            uint32_t id;
            glm::vec4 __padding_id;
        };

        MaterialUboHeader() {}
    };

    struct MaterialUbo {
        MaterialUboHeader header;
        glm::uvec4 data[8];

        MaterialUbo() {}
    };

    /// Mesh UBO used by GBuffer stage.
    struct MeshUbo {
        glm::mat4 transform;
    };
}
