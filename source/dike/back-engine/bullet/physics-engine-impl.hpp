#pragma once

#include <lava/dike/physics-engine.hpp>
#include <lava/dike/rigid-body.hpp>

namespace lava::dike {
    class PhysicsEngine::Impl {
    public:
        Impl() {}

        // PhysicsEngine
        void update(float dt);
        void gravity(const glm::vec3& gravity);

        // Adders
        void add(std::unique_ptr<RigidBody>&& rigidBody);
        void remove(const RigidBody& rigidBody);

        // Getters
        btDiscreteDynamicsWorld& dynamicsWorld() { return m_dynamicsWorld; }
        const btDiscreteDynamicsWorld& dynamicsWorld() const { return m_dynamicsWorld; }

    private:
        // Physics world
        btDefaultCollisionConfiguration m_collisionConfiguration;
        btCollisionDispatcher m_dispatcher{&m_collisionConfiguration};
        btDbvtBroadphase m_broadphase;
        btSequentialImpulseConstraintSolver m_solver;
        btDiscreteDynamicsWorld m_dynamicsWorld{&m_dispatcher, &m_broadphase, &m_solver, &m_collisionConfiguration};

        // Resources
        std::vector<std::unique_ptr<RigidBody>> m_rigidBodies;
    };
}
