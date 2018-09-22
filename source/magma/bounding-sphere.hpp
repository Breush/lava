#pragma once

#include <glm/glm.hpp>

namespace lava::magma {
    struct BoundingSphere {
        glm::vec3 center = glm::vec3(0.f);
        float radius = 0.f;
    };
}
