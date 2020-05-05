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
        void addMeshShape(const glm::mat4& localTransform, VectorView<glm::vec3> vertices, const std::vector<uint16_t>& indices);

        // Physics world
        bool enabled() const { return m_enabled; }
        void enabled(bool enabled);

        bool dynamic() const { return m_dynamic; }
        void dynamic(bool dynamic);

        void resetTransformChanged() { m_motionState.resetTransformChanged(); }
        bool transformChanged() const { return m_enabled && m_dynamic && m_motionState.transformChanged(); }

        const lava::Transform& transform() const { return m_transform; }
        void transform(const lava::Transform& transform);

        float distanceFrom(const Ray& ray, float maxDistance) const;

    protected:
        // Internal
        void addShape(const glm::mat4& localTransform, std::unique_ptr<btCollisionShape>&& pShape);
        void updateShape();

    private:
        PhysicsEngine::Impl& m_engine;

        bool m_enabled = true;
        bool m_dynamic = true;
        btScalar m_mass = 1.f;
        btVector3 m_inertia{0.f, 0.f, 0.f};

        // @note This transform will always be up-to-date
        // thanks to the motion state bindings.
        lava::Transform m_transform;
        MotionState m_motionState{m_transform};

        std::vector<std::unique_ptr<btTriangleIndexVertexArray>> m_meshShapeArrays;

        btCompoundShape m_shape;
        std::vector<std::unique_ptr<btCollisionShape>> m_shapes;
        std::unique_ptr<btRigidBody> m_rigidBody;
    };
}
