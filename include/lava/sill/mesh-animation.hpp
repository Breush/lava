#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <lava/core/interpolation-type.hpp>

namespace lava::sill {
    /// Defines which component of the mesh's node should be animated.
    enum class MeshAnimationPath {
        Unknown,
        Translation,
        Rotation,
        Scaling,
    };

    /**
     * An animation can contain multiple channels,
     * and they all follow the very same timer.
     */
    struct MeshAnimationChannel {
        std::vector<float> timeSteps;

        InterpolationType interpolationType = InterpolationType::Unknown; // Tells us how it is animated.
        MeshAnimationPath path = MeshAnimationPath::Unknown;              // Tells us what is animated.
        std::vector<glm::vec3> translation;
        std::vector<glm::quat> rotation;
        std::vector<glm::vec3> scaling;
    };

    /**
     * An animation can be defined for multiple nodeIndices.
     * And for each of these, they can have their own animation channels.
     * One map defined below has one global timer.
     */
    using MeshAnimation = std::unordered_map<uint32_t, std::vector<MeshAnimationChannel>>;
}
