#include "./physics-engine-impl.hpp"

using namespace lava::dike;

PhysicsEngine::Impl::Impl()
{
}

void PhysicsEngine::Impl::update(float dt)
{
    m_dynamicsWorld.stepSimulation(dt);
}

void PhysicsEngine::Impl::gravity(const glm::vec3& gravity)
{
    m_dynamicsWorld.setGravity({gravity.x, gravity.y, gravity.z});
}

void PhysicsEngine::Impl::add(std::unique_ptr<IStaticRigidBody>&& staticRigidBody)
{
    m_staticRigidBodies.emplace_back(std::move(staticRigidBody));
}

void PhysicsEngine::Impl::add(std::unique_ptr<IRigidBody>&& rigidBody)
{
    m_rigidBodies.emplace_back(std::move(rigidBody));
}
