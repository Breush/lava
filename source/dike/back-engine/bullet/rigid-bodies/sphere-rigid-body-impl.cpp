#include "./sphere-rigid-body-impl.hpp"

#include "../physics-engine-impl.hpp"

using namespace lava::dike;

SphereRigidBody::Impl::Impl(PhysicsEngine& engine, float diameter)
    : m_engine(engine.impl())
    , m_shape(diameter / 2.f) // @note btSphereShape takes radius
{
    if (m_mass > 0.f) {
        m_shape.calculateLocalInertia(m_mass, m_inertia);
    }

    btRigidBody::btRigidBodyConstructionInfo constructionInfo(m_mass, &m_motionState, &m_shape, m_inertia);
    constructionInfo.m_restitution = 0.5f;
    m_rigidBody = std::make_unique<btRigidBody>(constructionInfo);
    m_engine.dynamicsWorld().addRigidBody(m_rigidBody.get());
}

void SphereRigidBody::Impl::enabled(bool enabled)
{
    if (m_enabled == enabled) return;
    m_enabled = enabled;

    if (!enabled) {
        m_engine.dynamicsWorld().removeRigidBody(m_rigidBody.get());
    }
    else {
        m_engine.dynamicsWorld().addRigidBody(m_rigidBody.get());
    }
}

void SphereRigidBody::Impl::transform(const glm::mat4& transform)
{
    m_transform = transform;

    btTransform worldTransform;
    worldTransform.setFromOpenGLMatrix(reinterpret_cast<float*>(&m_transform));

    m_rigidBody->setWorldTransform(worldTransform);
    m_motionState.setWorldTransform(worldTransform);

    m_rigidBody->activate(true);
    m_rigidBody->clearForces();
    m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));
    m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));
}

void SphereRigidBody::Impl::translate(const glm::vec3& delta)
{
    // @fixme All rigid bodies impl have the very same code,
    // we should be able to factorize things to a IRigidBodyImpl class.

    transform(glm::translate(m_transform, delta));
}
