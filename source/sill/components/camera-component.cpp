#include <lava/sill/components/camera-component.hpp>

#include <lava/core/macros.hpp>

#include "./camera-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(CameraComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(CameraComponent, void, update, float, dt);

// Public API
$pimpl_property(CameraComponent, glm::vec3, translation);
$pimpl_property(CameraComponent, glm::vec3, target);
$pimpl_property_v(CameraComponent, float, radius);

$pimpl_method(CameraComponent, void, strafe, float, x, float, y);
$pimpl_method(CameraComponent, void, radiusAdd, float, radiusDistance);
$pimpl_method(CameraComponent, void, orbitAdd, float, longitudeAngle, float, latitudeAngle);
