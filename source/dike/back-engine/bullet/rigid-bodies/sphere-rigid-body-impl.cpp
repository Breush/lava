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
}

glm::vec3 SphereRigidBody::Impl::translation() const
{
    return m_transform[3];
}

void SphereRigidBody::Impl::translate(const glm::vec3& delta)
{
    m_transform = glm::translate(m_transform, delta);

    btTransform worldTransform;
    worldTransform.setFromOpenGLMatrix(reinterpret_cast<float*>(&m_transform));

    m_rigidBody->setWorldTransform(worldTransform);
    m_motionState.setWorldTransform(worldTransform);

    m_rigidBody->activate(true);
    m_rigidBody->clearForces();
    m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));
    m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));
}
