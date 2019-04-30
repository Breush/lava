#pragma once

#include <lava/dike/physics-engine.hpp>
#include <lava/dike/rigid-bodies/box-rigid-body.hpp>

#include "../motion-state.hpp"

namespace lava::dike {
    class BoxRigidBody::Impl {
    public:
        Impl(PhysicsEngine& engine, const glm::vec3& dimensions);

        void dimensions(const glm::vec3& dimensions);

        // IRigidBody
        bool enabled() const { return m_enabled; }
        void enabled(bool enabled);

        const glm::mat4& transform() const { return m_transform; }
        void transform(const glm::mat4& transform);

        glm::vec3 translation() const { return m_transform[3]; }
        void translate(const glm::vec3& delta);

    private:
        PhysicsEngine::Impl& m_engine;

        bool m_enabled = true;
        btScalar m_mass = 1.f;
        btVector3 m_inertia{0.f, 0.f, 0.f};

        // @note This transform will always be up-to-date
        // thanks to the motion state bindings.
        glm::mat4 m_transform = glm::mat4(1.f);
        MotionState m_motionState{m_transform};

        btBoxShape m_shape;
        std::unique_ptr<btRigidBody> m_rigidBody;
    };
}
