#include "./physics-engine-impl.hpp"

#include "./rigid-body-impl.hpp"

using namespace lava::dike;

void PhysicsEngine::Impl::update(float dt)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    for (auto& rigidBody : m_rigidBodies) {
        rigidBody->impl().resetTransformChanged();
    }

    m_dynamicsWorld.stepSimulation(dt);
}

void PhysicsEngine::Impl::gravity(const glm::vec3& gravity)
{
    m_dynamicsWorld.setGravity({gravity.x, gravity.y, gravity.z});
}

void PhysicsEngine::Impl::add(std::unique_ptr<RigidBody>&& rigidBody)
{
    m_rigidBodies.emplace_back(std::move(rigidBody));
}

void PhysicsEngine::Impl::remove(const RigidBody& rigidBody)
{
    for (auto iRigidBody = m_rigidBodies.begin(); iRigidBody != m_rigidBodies.end(); iRigidBody++) {
        if (iRigidBody->get() == &rigidBody) {
            m_rigidBodies.erase(iRigidBody);
            break;
        }
    }
}
