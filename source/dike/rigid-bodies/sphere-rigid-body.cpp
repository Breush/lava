#include <lava/dike/rigid-bodies/sphere-rigid-body.hpp>

#include <lava/core/macros.hpp>

#include "../back-engine/bullet/rigid-bodies/sphere-rigid-body-impl.hpp"

using namespace lava::dike;

$pimpl_class(SphereRigidBody, PhysicsEngine&, engine, float, radius);

$pimpl_method_const(SphereRigidBody, glm::vec3, position);
$pimpl_method(SphereRigidBody, void, positionAdd, const glm::vec3&, delta);
