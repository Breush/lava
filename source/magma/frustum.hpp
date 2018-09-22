#pragma once

#include <glm/glm.hpp>

namespace lava::magma {
    /**
     * A frustum, as composed of planes, delimiting half-space,
     * and a forward normalized vector, with near and far components.
     *
     * Vectors are composed of:
     * - sideNormal   Normal, going outwards the pyramid;
     * - sideDistance Distance, of the plane from the origin.
     */
    struct Frustum {
        glm::vec3 leftNormal;
        float leftDistance;
        glm::vec3 rightNormal;
        float rightDistance;
        glm::vec3 topNormal;
        float topDistance;
        glm::vec3 bottomNormal;
        float bottomDistance;

        glm::vec3 forward;
        float near;
        float far;
    };
}
