#include "./plane-static-rigid-body-impl.hpp"

#include "../physics-engine-impl.hpp"

using namespace lava::dike;

PlaneStaticRigidBody::Impl::Impl(PhysicsEngine& engine, const glm::vec3& normal)
    : m_shape(btVector3(normal.x, normal.y, normal.z), 0)
    , m_rigidBody(btRigidBody::btRigidBodyConstructionInfo(0.f, &m_motionState, &m_shape, btVector3(0, 0, 0)))
{
    engine.impl().dynamicsWorld().addRigidBody(&m_rigidBody);
}
