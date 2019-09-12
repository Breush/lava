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
        auto centerDiff = bs2.center - bs1.center;
        auto centerDistanceSquared = glm::dot(centerDiff, centerDiff);
        if (centerDistanceSquared <= bs1.radius * bs1.radius) return bs1;
        if (centerDistanceSquared <= bs2.radius * bs2.radius) return bs2;

        // Otherwise, just a simple formula.
        auto centerDistance = std::sqrt(centerDistanceSquared);
        BoundingSphere bs;
        bs.radius = (bs1.radius + bs2.radius + centerDistance) / 2.f;
        bs.center = bs1.center + centerDiff * (bs.radius - bs1.radius) / centerDistance;

        return bs;
    }
}
