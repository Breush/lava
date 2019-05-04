#include <lava/sill/components/camera-component.hpp>

#include "./camera-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(CameraComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(CameraComponent, void, update, float, dt);

// Public API
$pimpl_method_const(CameraComponent, const lava::Extent2d&, extent);

$pimpl_property(CameraComponent, glm::vec3, translation);
$pimpl_property(CameraComponent, glm::vec3, target);
$pimpl_property_v(CameraComponent, float, radius);

$pimpl_method(CameraComponent, void, strafe, float, x, float, y);
$pimpl_method(CameraComponent, void, radiusAdd, float, radiusDistance);
$pimpl_method(CameraComponent, void, orbitAdd, float, longitudeAngle, float, latitudeAngle);

$pimpl_method_const(CameraComponent, const glm::mat4&, viewTransform);
$pimpl_method_const(CameraComponent, const glm::mat4&, projectionTransform);

$pimpl_method_const(CameraComponent, lava::Ray, coordinatesToRay, const glm::vec2&, coordinates);
$pimpl_method_const(CameraComponent, glm::vec3, unproject, const glm::vec2&, coordinates, float, depth);

glm::mat4 CameraComponent::transformAtCoordinates(const glm::vec2& coordinates, float depth) const
{
    auto targetFront = unproject(coordinates, depth);
    auto furtherFront = unproject(coordinates, 0.f);
    auto targetLeft = unproject(coordinates - glm::vec2{0.5f, 0.f}, depth);
    auto targetUp = unproject(coordinates - glm::vec2{0.f, 0.5f}, depth);

    auto front = glm::normalize(furtherFront - targetFront);
    auto left = glm::normalize(targetLeft - targetFront);
    auto up = glm::normalize(targetUp - targetFront);

    auto transform = glm::mat4(1.f);
    transform[0] = glm::vec4(front, 0.f);
    transform[1] = glm::vec4(left, 0.f);
    transform[2] = glm::vec4(up, 0.f);
    transform[3] = glm::vec4(targetFront, 1.f);

    return transform;
}
