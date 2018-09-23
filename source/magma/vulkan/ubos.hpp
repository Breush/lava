#pragma once

#include "../light-type.hpp"

// @fixme UBO definitions should not be in 'vulkan' namespace...
namespace lava::magma::vulkan {
    struct CameraUbo {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 wPosition;
        glm::uvec2 extent;
    };

    struct MaterialUboHeader {
        union {
            uint32_t id;
            glm::vec4 __padding;
        };

        MaterialUboHeader() {}
    };

    struct MaterialUbo {
        MaterialUboHeader header;
        glm::uvec4 data[8];

        MaterialUbo() {}
    };

    struct MeshUbo {
        glm::mat4 transform;
    };

    struct LightUbo {
        union {
            uint32_t type;
            glm::vec4 __padding;
        };

        glm::mat4 transform;
        glm::uvec4 data[2];

        LightUbo(LightType lightType)
            : type(static_cast<uint32_t>(lightType))
        {
        }
    };
}
