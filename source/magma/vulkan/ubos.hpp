#pragma once

namespace lava::magma::vulkan {
    /// Camera UBO used by GBuffer stage.
    constexpr static const auto CAMERA_DESCRIPTOR_SET_INDEX = 0u;
    struct CameraUbo {
        glm::mat4 view;
        glm::mat4 projection;
    };

    /// Material UBO used by GBuffer stage.
    constexpr static const auto MATERIAL_DESCRIPTOR_SET_INDEX = 1u;
    struct MaterialUbo {
        float roughnessFactor;
        float metallicFactor;
    };

    /// Mesh UBO used by GBuffer stage.
    constexpr static const auto MESH_DESCRIPTOR_SET_INDEX = 2u;
    struct MeshUbo {
        glm::mat4 transform;
    };
}
