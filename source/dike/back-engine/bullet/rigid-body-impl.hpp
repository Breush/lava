#pragma once

#include <lava/dike/physics-engine.hpp>
#include <lava/dike/rigid-body.hpp>

#include "./motion-state.hpp"

namespace lava::dike {
    class RigidBody::Impl {
    public:
        Impl(PhysicsEngine& engine);
        ~Impl();

        // Shapes
        void clearShapes();
        void addBoxShape(const glm::vec3& offset, const glm::vec3& dimensions);
        void addSphereShape(const glm::vec3& offset, float diameter);
        void addInfinitePlaneShape(const glm::vec3& offset, const glm::vec3& normal);

        // Physics world
        bool enabled() const { return m_enabled; }
        void enabled(bool enabled);

        bool dynamic() const { return m_dynamic; }
        void dynamic(bool dynamic);

        void resetTransformChanged() { m_motionState.resetTransformChanged(); }
        bool transformChanged() const { return m_enabled && m_dynamic && m_motionState.transformChanged(); }

        const glm::mat4& transform() const { return m_transform; }
        void transform(const glm::mat4& transform);

    protected:
        // Internal
        void addShape(const glm::vec3& offset, std::unique_ptr<btCollisionShape>&& pShape);
        void updateShape();

    private:
        PhysicsEngine::Impl& m_engine;

        bool m_enabled = true;
        bool m_dynamic = true;
        btScalar m_mass = 1.f;
        btVector3 m_inertia{0.f, 0.f, 0.f};

        // @note This transform will always be up-to-date
        // thanks to the motion state bindings.
        glm::mat4 m_transform = glm::mat4(1.f);
        glm::vec3 m_translation = glm::vec3{0.f};
        glm::vec3 m_scaling = glm::vec3{1.f};
        glm::quat m_rotation = glm::quat{1.f, 0.f, 0.f, 0.f};
        MotionState m_motionState{m_transform, m_translation, m_rotation, m_scaling};

        btCompoundShape m_shape;
        std::vector<std::unique_ptr<btCollisionShape>> m_shapes;
        std::unique_ptr<btRigidBody> m_rigidBody;
    };
}
