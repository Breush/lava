#pragma once

#include "../bounding-sphere.hpp"
#include "../frustum.hpp"

namespace lava::magma::helpers {
    /// Checks whether the bounding sphere is at least partly inside the frustum.
    inline bool isVisibleInsideFrustum(const BoundingSphere& boundingSphere, const Frustum& frustum)
    {
        auto forwardDistance = glm::dot(boundingSphere.center, frustum.forward);
        return (forwardDistance + boundingSphere.radius >= frustum.near)
               && (forwardDistance - boundingSphere.radius <= frustum.far)
               && (glm::dot(boundingSphere.center, frustum.leftNormal) - boundingSphere.radius <= frustum.leftDistance)
               && (glm::dot(boundingSphere.center, frustum.rightNormal) - boundingSphere.radius <= frustum.rightDistance)
               && (glm::dot(boundingSphere.center, frustum.bottomNormal) - boundingSphere.radius <= frustum.bottomDistance)
               && (glm::dot(boundingSphere.center, frustum.topNormal) - boundingSphere.radius <= frustum.topDistance);
    }
}
