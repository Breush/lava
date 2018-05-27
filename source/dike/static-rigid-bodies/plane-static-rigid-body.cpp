#include <lava/dike/static-rigid-bodies/plane-static-rigid-body.hpp>

#include <lava/core/macros/pimpl.hpp>

#include "../back-engine/bullet/static-rigid-bodies/plane-static-rigid-body-impl.hpp"

using namespace lava::dike;

$pimpl_class(PlaneStaticRigidBody, PhysicsEngine&, engine, const glm::vec3&, normal);
