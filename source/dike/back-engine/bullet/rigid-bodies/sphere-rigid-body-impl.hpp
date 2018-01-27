#pragma once

#include <lava/dike/physics-engine.hpp>
#include <lava/dike/rigid-bodies/sphere-rigid-body.hpp>

#include <bullet/btBulletDynamicsCommon.h>

namespace lava::dike {
    class SphereRigidBody::Impl {
    public:
        Impl(PhysicsEngine& engine, float radius);

        // IRigidBody
        glm::vec3 position() const;
        void positionAdd(const glm::vec3& delta);

    protected:
        void reconstructRigidBody();

    private:
        PhysicsEngine::Impl& m_engine;

        btScalar m_mass = 1.f;
        btVector3 m_inertia;

        btSphereShape m_shape;
        btDefaultMotionState m_motionState;
        btRigidBody m_rigidBody;
        btTransform m_transform;
    };
}
