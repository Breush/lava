#include <lava/dike/rigid-body.hpp>

#include "./back-engine/bullet/rigid-body-impl.hpp"

using namespace lava::dike;

$pimpl_class(RigidBody, PhysicsEngine&, engine);

// Shapes
$pimpl_method(RigidBody, void, clearShapes);
$pimpl_method(RigidBody, void, addBoxShape, const glm::vec3&, offset, const glm::vec3&, dimensions);
$pimpl_method(RigidBody, void, addSphereShape, const glm::vec3&, offset, float, diameter);
$pimpl_method(RigidBody, void, addInfinitePlaneShape, const glm::vec3&, offset, const glm::vec3&, normal);

// Physics world
$pimpl_method_const(RigidBody, bool, enabled);
$pimpl_method(RigidBody, void, enabled, bool, enabled);

$pimpl_method_const(RigidBody, bool, dynamic);
$pimpl_method(RigidBody, void, dynamic, bool, dynamic);

$pimpl_method_const(RigidBody, const glm::mat4&, transform);
$pimpl_method(RigidBody, void, transform, const glm::mat4&, transform);
