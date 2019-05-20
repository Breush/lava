#pragma once

#include "../light-type.hpp"

namespace lava::magma {
    constexpr const auto MATERIAL_DATA_SIZE = 16u;
    constexpr const auto MATERIAL_SAMPLERS_SIZE = 8u;
}

// @fixme UBO definitions should not be in 'vulkan' namespace...
namespace lava::magma::vulkan {
    struct CameraUbo {                // 80 bytes
        glm::vec4 viewTransform0;     // 16 transpose(viewTransform)[0]
        glm::vec4 viewTransform1;     // 16 transpose(viewTransform)[1]
        glm::vec4 viewTransform2;     // 16 transpose(viewTransform)[2]
        glm::vec4 projectionFactors0; // 16 [0][0] [1][1] [2][2] [3][2]
        glm::vec4 projectionFactors1; // 16 [2][0] [2][1] extent.width extent.height
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

    // @note Transform is decomposed into the three first lines of the transpose matrix
    // because it is sent via push constants and we're limited to 128 bytes max.
    // Doing that, we make place for the camera ubo too.
    struct MeshUbo {          // 48 bytes
        glm::vec4 transform0; // 16 transpose(transform)[0]
        glm::vec4 transform1; // 16 transpose(transform)[1]
        glm::vec4 transform2; // 16 transpose(transform)[2]
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
