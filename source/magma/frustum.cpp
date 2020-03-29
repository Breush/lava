#include <lava/magma/frustum.hpp>

using namespace lava::magma;

bool Frustum::canSee(const glm::vec3& point) const
{
    auto forwardDistance = glm::dot(point, forward);
    return (forwardDistance >= near) &&
           (forwardDistance <= far) &&
           (glm::dot(point, leftNormal) <= leftDistance) &&
           (glm::dot(point, rightNormal) <= rightDistance) &&
           (glm::dot(point, bottomNormal) <= bottomDistance) &&
           (glm::dot(point, topNormal) <= topDistance);
}

bool Frustum::canSee(const BoundingSphere& boundingSphere) const
{
    auto forwardDistance = glm::dot(boundingSphere.center, forward);
    return (forwardDistance + boundingSphere.radius >= near) &&
           (forwardDistance - boundingSphere.radius <= far) &&
           (glm::dot(boundingSphere.center, leftNormal) - boundingSphere.radius <= leftDistance) &&
           (glm::dot(boundingSphere.center, rightNormal) - boundingSphere.radius <= rightDistance) &&
           (glm::dot(boundingSphere.center, bottomNormal) - boundingSphere.radius <= bottomDistance) &&
           (glm::dot(boundingSphere.center, topNormal) - boundingSphere.radius <= topDistance);
}
