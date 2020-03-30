#pragma once

#include <glm/vec3.hpp>

namespace lava {
    struct BoundingSphere {
        glm::vec3 center = glm::vec3(0.f);
        float radius = 0.f;
    };

    // Find the bounding sphere that encapsulate both bounding spheres.
    inline BoundingSphere mergeBoundingSpheres(const BoundingSphere& bs1, const BoundingSphere& bs2)
    {
        // @note Very small bounding spheres are considered as null.
        if (bs1.radius <= 0.0001f) return bs2;
        if (bs2.radius <= 0.0001f) return bs1;

        // Check if a sphere encloses another one, return it if so.
        auto c12 = bs2.center - bs1.center;
        auto d = glm::length(c12); // Distance between centers
        if (d + bs1.radius <= bs2.radius) return bs2;
        if (d + bs2.radius <= bs1.radius) return bs1;

        // Otherwise, take the center and compute radius.
        BoundingSphere bs;
        bs.radius = (bs1.radius + d + bs2.radius) / 2.f;
        bs.center = bs1.center + c12 * (bs.radius - bs1.radius) / d;
        return bs;
    }
}
