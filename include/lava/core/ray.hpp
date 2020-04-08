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

    // Outputs the parametrization of the intersection of ray with the plane defined by its origin/normal.
    inline float intersectPlane(const Ray& ray, const Ray& planeRay)
    {
        float denom = glm::dot(planeRay.direction, ray.direction);
        if (glm::abs(denom) > 0.001f) {
            return glm::dot(planeRay.origin - ray.origin, planeRay.direction) / denom;
        }

        // Parallel
        return 0.f;
    }

    // Outputs the parametrization of the intersection of ray with the triangle.
    // Returns 0.f when not intersecting.
    // @note We're using algorithm from Real-Time Rendering Fourth Edition - page 965,
    // which projects the ray into the barycentric coordinates system of the triangle to test.
    inline float intersectTriangle(const Ray& ray, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
    {
        auto e01 = p1 - p0;
        auto e02 = p2 - p0;

        // Ignore triangles with opposite normals
        auto c = glm::cross(e01, e02);
        if (glm::dot(c, ray.direction) > 0.f) return 0.f;

        // Ray might be parallel to triangle
        auto q = glm::cross(ray.direction, e02);
        auto a = glm::dot(e01, q);
        if (a > -0.0001f && a < 0.0001f) return 0.f;

        auto f = 1.f / a;
        auto s = ray.origin - p0;
        auto u = f * glm::dot(s, q);
        if (u < 0.f) return 0.f;

        auto r = glm::cross(s, e01);
        auto v = f * glm::dot(ray.direction, r);
        if (v < 0.f || u + v > 1.f) return 0.f;

        auto t = f * glm::dot(e02, r);
        if (t < 0.f) return 0.f;

        return t;
    }

    // Outputs the parametrization of the intersection of ray with the sphere.
    // Returns 0.f when not intersecting.
    inline float intersectSphere(const Ray& ray, const glm::vec3& center, float radius)
    {
        auto originToCenter = center - ray.origin;
        auto t = glm::dot(ray.direction, originToCenter);
        if (t <= 0.f) return 0.f;

        auto intersectionPoint = ray.origin + ray.direction * t;
        if (glm::length(center - intersectionPoint) > radius) return 0.f;

        return t;
    }
}
