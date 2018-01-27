#include "./sphere-rigid-body-impl.hpp"

#include "../physics-engine-impl.hpp"

using namespace lava::dike;

SphereRigidBody::Impl::Impl(PhysicsEngine& engine, float radius)
    : m_engine(engine.impl())
    , m_shape(radius)
    , m_rigidBody(btRigidBody::btRigidBodyConstructionInfo(m_mass, &m_motionState, &m_shape, m_inertia))
{
    // @todo Have a custom motion state, binded with the transform,
    // and having a callback

    reconstructRigidBody();
}

glm::vec3 SphereRigidBody::Impl::position() const
{
    m_motionState.getWorldTransform(const_cast<btTransform&>(m_transform));
    auto position = m_transform.getOrigin();
    return {position.getX(), position.getY(), position.getZ()};
}

void SphereRigidBody::Impl::positionAdd(const glm::vec3& delta)
{
    m_transform.setOrigin({0.f, 0.f, 1.f});
    m_motionState.setWorldTransform(m_transform);

    reconstructRigidBody();
}

//----- Internal

void SphereRigidBody::Impl::reconstructRigidBody()
{
    // @todo Update local inertia according to the shape
    // m_shape.calculateLocalInertia(m_mass, m_inertia);

    // Remove and add the rigid body
    // @todo Why should it be necessary? It just holds a pointer!
    m_engine.dynamicsWorld().removeRigidBody(&m_rigidBody);
    btRigidBody::btRigidBodyConstructionInfo constructionInfo(m_mass, &m_motionState, &m_shape, m_inertia);
    m_rigidBody = constructionInfo;
    m_engine.dynamicsWorld().addRigidBody(&m_rigidBody);
}
