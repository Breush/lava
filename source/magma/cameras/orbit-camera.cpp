#include <lava/magma/cameras/orbit-camera.hpp>

#include <glm/gtx/rotate_vector.hpp>
#include <lava/chamber/macros.hpp>

#include "./orbit-camera-impl.hpp"

using namespace lava::magma;

$pimpl_class(OrbitCamera, RenderEngine&, engine);

// ICamera
$pimpl_method_const(OrbitCamera, const glm::vec3&, position);
$pimpl_method_const(OrbitCamera, const glm::mat4&, viewTransform);
$pimpl_method_const(OrbitCamera, const glm::mat4&, projectionTransform);

$pimpl_method(OrbitCamera, void, position, const glm::vec3&, position);
$pimpl_method(OrbitCamera, void, target, const glm::vec3&, target);
$pimpl_method(OrbitCamera, void, viewportRatio, float, viewportRatio);

void OrbitCamera::radius(float radius)
{
    const auto& position = m_impl->position();
    const auto& target = m_impl->target();
    auto relativePosition = position - target;
    m_impl->position(target + glm::normalize(relativePosition) * radius);
}

void OrbitCamera::strafe(float x, float y)
{
    const auto& position = m_impl->position();
    const auto& target = m_impl->target();
    auto viewDirection = target - position;
    auto radius = glm::length(viewDirection);

    const glm::vec3 up(0.f, 0.f, 1.f);
    auto normal = normalize(viewDirection);
    auto tangent = glm::cross(up, normal);
    tangent = normalize(tangent - normal * dot(normal, tangent)); // Orthogonalization
    auto bitangent = normalize(glm::cross(normal, tangent));

    glm::vec3 delta = (x * tangent + y * bitangent) * radius;
    m_impl->position(position + delta);
    m_impl->target(target + delta);
}

void OrbitCamera::radiusAdd(float radiusDistance)
{
    const auto& position = m_impl->position();
    const auto& target = m_impl->target();
    auto relativePosition = position - target;
    auto radius = glm::length(relativePosition) + radiusDistance;
    m_impl->position(target + glm::normalize(relativePosition) * radius);
}

void OrbitCamera::orbitAdd(float longitudeAngle, float latitudeAngle)
{
    const auto& position = m_impl->position();
    const auto& target = m_impl->target();
    auto relativePosition = position - target;
    auto axis = glm::vec3(-relativePosition.y, relativePosition.x, 0);

    auto longitudeDelta = glm::rotateZ(relativePosition, longitudeAngle) - relativePosition;
    auto latitudeDelta = glm::rotate(relativePosition, latitudeAngle, axis) - relativePosition;
    m_impl->position(position + longitudeDelta + latitudeDelta);
}
