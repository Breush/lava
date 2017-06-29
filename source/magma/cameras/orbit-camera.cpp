#include <lava/magma/cameras/orbit-camera.hpp>

#include <glm/gtx/rotate_vector.hpp>
#include <lava/chamber/pimpl.hpp>

#include "./orbit-camera-impl.hpp"

using namespace lava;

$pimpl_class(OrbitCamera, RenderEngine&, engine);

// ICamera
$pimpl_method_const(OrbitCamera, const glm::vec3&, position);
$pimpl_method_const(OrbitCamera, const glm::mat4&, viewTransform);
$pimpl_method_const(OrbitCamera, const glm::mat4&, projectionTransform);

$pimpl_method(OrbitCamera, void, position, const glm::vec3&, position);
$pimpl_method(OrbitCamera, void, target, const glm::vec3&, target);
$pimpl_method(OrbitCamera, void, viewportRatio, float, viewportRatio);

void OrbitCamera::latitudeAdd(float latitudeDelta)
{
    const auto& position = m_impl->position();
    const auto& target = m_impl->target();
    auto relativePosition = position - target;
    auto axis = glm::vec3(-relativePosition.y, relativePosition.x, 0);
    m_impl->position(target + glm::rotate(relativePosition, latitudeDelta, axis));
}

void OrbitCamera::longitudeAdd(float longitudeDelta)
{
    const auto& position = m_impl->position();
    const auto& target = m_impl->target();
    auto relativePosition = position - target;
    m_impl->position(target + glm::rotateZ(relativePosition, longitudeDelta));
}
