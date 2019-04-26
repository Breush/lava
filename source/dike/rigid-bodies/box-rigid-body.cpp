#include <lava/dike/rigid-bodies/box-rigid-body.hpp>

#include "../back-engine/bullet/rigid-bodies/box-rigid-body-impl.hpp"

using namespace lava::dike;

$pimpl_class(BoxRigidBody, PhysicsEngine&, engine, const glm::vec3&, dimensions);

$pimpl_method_const(BoxRigidBody, bool, enabled);
$pimpl_method(BoxRigidBody, void, enabled, bool, enabled);

$pimpl_method_const(BoxRigidBody, const glm::mat4&, transform);
$pimpl_method(BoxRigidBody, void, transform, const glm::mat4&, transform);

$pimpl_method_const(BoxRigidBody, glm::vec3, translation);
$pimpl_method(BoxRigidBody, void, translate, const glm::vec3&, delta);
