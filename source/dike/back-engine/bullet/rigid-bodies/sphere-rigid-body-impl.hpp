#pragma once

#include <lava/dike/physics-engine.hpp>
#include <lava/dike/rigid-bodies/sphere-rigid-body.hpp>

#include "../motion-state.hpp"

namespace lava::dike {
    class SphereRigidBody::Impl {
    public:
        Impl(PhysicsEngine& engine, float radius);

        // IRigidBody
        glm::vec3 translation() const;
        void translate(const glm::vec3& delta);

    private:
        PhysicsEngine::Impl& m_engine;

        btScalar m_mass = 1.f;
        btVector3 m_inertia{0.f, 0.f, 0.f};

        // @note This transform will always be up-to-date
        // thanks to the motion state bindings.
        glm::mat4 m_transform;
        MotionState m_motionState{m_transform};

        btSphereShape m_shape;
        std::unique_ptr<btRigidBody> m_rigidBody;
    };
}
