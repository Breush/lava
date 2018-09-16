#include <lava/magma/cameras/orbit-camera.hpp>

#include <glm/gtx/rotate_vector.hpp>
#include <lava/chamber/math.hpp>
#include <lava/core/macros.hpp>

#include "../vulkan/cameras/orbit-camera-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

$pimpl_class(OrbitCamera, RenderScene&, scene, Extent2d, extent);

// ICamera
$pimpl_property_v(OrbitCamera, lava::Extent2d, extent);
$pimpl_method_const(OrbitCamera, RenderImage, renderImage);
$pimpl_method_const(OrbitCamera, RenderImage, depthRenderImage);
$pimpl_property_v(OrbitCamera, PolygonMode, polygonMode);

ICamera::Impl& OrbitCamera::interfaceImpl()
{
    return *m_impl;
}

$pimpl_property(OrbitCamera, glm::vec3, translation);
$pimpl_property(OrbitCamera, glm::vec3, target);

float OrbitCamera::radius() const
{
    return glm::length(m_impl->target() - m_impl->translation());
}

void OrbitCamera::radius(float radius)
{
    const auto& translation = m_impl->translation();
    const auto& target = m_impl->target();
    auto relativePosition = translation - target;
    m_impl->translation(target + glm::normalize(relativePosition) * radius);
}

void OrbitCamera::strafe(float x, float y)
{
    const auto& translation = m_impl->translation();
    const auto& target = m_impl->target();
    auto viewDirection = target - translation;
    auto radius = glm::length(viewDirection);

    const glm::vec3 up(0.f, 0.f, 1.f);
    auto normal = normalize(viewDirection);
    auto tangent = glm::cross(up, normal);
    tangent = normalize(tangent - normal * dot(normal, tangent)); // Orthogonalization
    auto bitangent = normalize(glm::cross(normal, tangent));

    glm::vec3 delta = (x * tangent + y * bitangent) * radius;
    m_impl->translation(translation + delta);
    m_impl->target(target + delta);
}

void OrbitCamera::radiusAdd(float radiusDistance)
{
    const auto& translation = m_impl->translation();
    const auto& target = m_impl->target();
    auto relativePosition = translation - target;
    auto radius = glm::length(relativePosition) + radiusDistance;
    radius = std::max(0.01f, radius);

    m_impl->translation(target + glm::normalize(relativePosition) * radius);
}

void OrbitCamera::orbitAdd(float longitudeAngle, float latitudeAngle)
{
    const auto& translation = m_impl->translation();
    const auto& target = m_impl->target();
    auto relativePosition = translation - target;
    auto axis = glm::vec3(relativePosition.y, -relativePosition.x, 0);

    auto currentLatitudeAngle = std::asin(relativePosition.z / glm::length(relativePosition));
    if (currentLatitudeAngle + latitudeAngle > math::PI_OVER_TWO - 0.01) {
        latitudeAngle = math::PI_OVER_TWO - 0.01 - currentLatitudeAngle;
    }
    else if (currentLatitudeAngle + latitudeAngle < -math::PI_OVER_TWO + 0.01) {
        latitudeAngle = -math::PI_OVER_TWO + 0.01 - currentLatitudeAngle;
    }

    auto longitudeDelta = glm::rotateZ(relativePosition, longitudeAngle) - relativePosition;
    auto latitudeDelta = glm::rotate(relativePosition, latitudeAngle, axis) - relativePosition;
    m_impl->translation(translation + longitudeDelta + latitudeDelta);
}
