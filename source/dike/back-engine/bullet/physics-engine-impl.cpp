#include "./physics-engine-impl.hpp"

using namespace lava::dike;

PhysicsEngine::Impl::Impl() {}

void PhysicsEngine::Impl::update(float dt)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

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

void PhysicsEngine::Impl::remove(const IStaticRigidBody& rigidBody)
{
    for (auto iRigidBody = m_staticRigidBodies.begin(); iRigidBody != m_staticRigidBodies.end(); iRigidBody++) {
        if (iRigidBody->get() == &rigidBody) {
            m_staticRigidBodies.erase(iRigidBody);
            break;
        }
    }
}

void PhysicsEngine::Impl::remove(const IRigidBody& rigidBody)
{
    for (auto iRigidBody = m_rigidBodies.begin(); iRigidBody != m_rigidBodies.end(); iRigidBody++) {
        if (iRigidBody->get() == &rigidBody) {
            m_rigidBodies.erase(iRigidBody);
            break;
        }
    }
}
