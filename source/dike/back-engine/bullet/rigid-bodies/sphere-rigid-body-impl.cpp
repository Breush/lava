#include "./sphere-rigid-body-impl.hpp"

#include "../physics-engine-impl.hpp"

using namespace lava::dike;

SphereRigidBody::Impl::Impl(PhysicsEngine& engine, float radius)
    : m_engine(engine.impl())
    , m_shape(radius)
{
    if (m_mass > 0.f) {
        m_shape.calculateLocalInertia(m_mass, m_inertia);
    }

    btRigidBody::btRigidBodyConstructionInfo constructionInfo(m_mass, &m_motionState, &m_shape, m_inertia);
    constructionInfo.m_restitution = 0.5f;
    m_rigidBody = std::make_unique<btRigidBody>(constructionInfo);
    m_engine.dynamicsWorld().addRigidBody(m_rigidBody.get());

    // @todo Have a custom motion state, binded with the transform,
    // and having a callback
}

glm::vec3 SphereRigidBody::Impl::translation() const
{
    m_motionState.getWorldTransform(const_cast<btTransform&>(m_transform));
    const auto& translation = m_transform.getOrigin();
    return {translation.getX(), translation.getY(), translation.getZ()};
}

void SphereRigidBody::Impl::translate(const glm::vec3& delta)
{
    auto origin = m_transform.getOrigin() + btVector3(delta.x, delta.y, delta.z);
    m_transform.setOrigin(origin);
    m_motionState.setWorldTransform(m_transform);
    m_rigidBody->setWorldTransform(m_transform);
}
