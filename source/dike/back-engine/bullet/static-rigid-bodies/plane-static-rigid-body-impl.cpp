#include "./plane-static-rigid-body-impl.hpp"

#include "../physics-engine-impl.hpp"

using namespace lava::dike;

PlaneStaticRigidBody::Impl::Impl(PhysicsEngine& engine, const glm::vec3& normal)
    : m_shape(btVector3(normal.x, normal.y, normal.z), 0)
    , m_constructionInfo(0.f, nullptr, &m_shape)
{
    m_constructionInfo.m_restitution = 0.5f;
    m_rigidBody = std::make_unique<btRigidBody>(m_constructionInfo);

    engine.impl().dynamicsWorld().addRigidBody(m_rigidBody.get());
}

PlaneStaticRigidBody::Impl::~Impl()
{
    // @todo Delete rigid body!
}
