#pragma once

#include "../light-type.hpp"

namespace lava::magma {
    constexpr const auto MATERIAL_DATA_SIZE = 16u;
    constexpr const auto MATERIAL_SAMPLERS_SIZE = 8u;
}

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
        glm::uvec4 data[MATERIAL_DATA_SIZE];

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
