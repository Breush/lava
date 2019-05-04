#pragma once

#include <glm/glm.hpp>

namespace lava {
    /// A 3D ray used for ray picking/casting.
    struct Ray {
        glm::vec3 origin = {0.f, 0.f, 0.f};
        glm::vec3 direction = {1.f, 0.f, 0.f};
    };
}
