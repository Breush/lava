#include "./box-rigid-body-impl.hpp"

#include "../physics-engine-impl.hpp"

using namespace lava::dike;

BoxRigidBody::Impl::Impl(PhysicsEngine& engine, const glm::vec3& dimensions)
    : m_engine(engine.impl())
    , m_shape(btVector3{dimensions.x / 2, dimensions.y / 2, dimensions.z / 2}) // @note btBoxShape takes halfExtent
{
    if (m_mass > 0.f) {
        m_shape.calculateLocalInertia(m_mass, m_inertia);
    }

    btRigidBody::btRigidBodyConstructionInfo constructionInfo(m_mass, &m_motionState, &m_shape, m_inertia);
    constructionInfo.m_restitution = 0.5f;
    m_rigidBody = std::make_unique<btRigidBody>(constructionInfo);
    m_engine.dynamicsWorld().addRigidBody(m_rigidBody.get());
}

void BoxRigidBody::Impl::transform(const glm::mat4& transform)
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

void BoxRigidBody::Impl::translate(const glm::vec3& delta)
{
    // @fixme All rigid bodies impl have the very same code,
    // we should be able to factorize things to a IRigidBodyImpl class.

    transform(glm::translate(m_transform, delta));
}
