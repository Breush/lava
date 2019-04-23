#include <lava/dike/rigid-bodies/sphere-rigid-body.hpp>

#include "../back-engine/bullet/rigid-bodies/sphere-rigid-body-impl.hpp"

using namespace lava::dike;

$pimpl_class(SphereRigidBody, PhysicsEngine&, engine, float, diameter);

$pimpl_method_const(SphereRigidBody, const glm::mat4&, transform);
$pimpl_method(SphereRigidBody, void, transform, const glm::mat4&, transform);

$pimpl_method_const(SphereRigidBody, glm::vec3, translation);
$pimpl_method(SphereRigidBody, void, translate, const glm::vec3&, delta);
