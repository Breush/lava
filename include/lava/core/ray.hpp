#pragma once

#include <glm/glm.hpp>

namespace lava {
    /// A 3D ray used for ray picking/casting.
    struct Ray {
        glm::vec3 origin = {0.f, 0.f, 0.f};
        glm::vec3 direction = {1.f, 0.f, 0.f};
    };

    // Outputs the parametrization of the projection of the closest point of projectedRay onto baseRay.
    inline float projectOn(const Ray& baseRay, const Ray& projectedRay)
    {
        const auto dp = projectedRay.origin - baseRay.origin;
        const auto v12 = glm::dot(baseRay.direction, baseRay.direction);
        const auto v22 = glm::dot(projectedRay.direction, projectedRay.direction);
        const auto v1v2 = glm::dot(baseRay.direction, projectedRay.direction);

        const auto det = v1v2 * v1v2 - v12 * v22;

        if (std::abs(det) > 0.01f) {
            const auto inv_det = 1.f / det;

            const auto dpv1 = dot(dp, baseRay.direction);
            const auto dpv2 = dot(dp, projectedRay.direction);

            const auto baseRayt = inv_det * (v22 * dpv1 - v1v2 * dpv2);

            return baseRayt;
        }
        else {
            // Lines are parallel.
            return 0.f;
        }
    }
}
