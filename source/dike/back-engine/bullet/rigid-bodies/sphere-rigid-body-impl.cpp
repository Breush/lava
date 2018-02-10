#include "./sphere-rigid-body-impl.hpp"

#include "../physics-engine-impl.hpp"

using namespace lava::dike;

SphereRigidBody::Impl::Impl(PhysicsEngine& engine, float radius)
    : m_engine(engine.impl())
    , m_shape(radius)
    , m_constructionInfo(m_mass, &m_motionState, &m_shape, m_inertia)
{
    m_constructionInfo.m_restitution = 0.5f;
    m_rigidBody = std::make_unique<btRigidBody>(m_constructionInfo);

    // @todo Have a custom motion state, binded with the transform,
    // and having a callback

    reconstructRigidBody();
}

glm::vec3 SphereRigidBody::Impl::position() const
{
    m_motionState.getWorldTransform(const_cast<btTransform&>(m_transform));
    const auto& position = m_transform.getOrigin();
    return {position.getX(), position.getY(), position.getZ()};
}

void SphereRigidBody::Impl::positionAdd(const glm::vec3& delta)
{
    auto origin = m_transform.getOrigin() + btVector3(delta.x, delta.y, delta.z);
    m_transform.setOrigin(origin);
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
    m_engine.dynamicsWorld().removeRigidBody(m_rigidBody.get());
    m_constructionInfo = btRigidBody::btRigidBodyConstructionInfo(m_mass, &m_motionState, &m_shape, m_inertia);
    m_constructionInfo.m_restitution = 0.5f;
    // @todo Bounciness in settings

    m_rigidBody = std::make_unique<btRigidBody>(m_constructionInfo);
    m_engine.dynamicsWorld().addRigidBody(m_rigidBody.get());
}
