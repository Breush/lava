#pragma once

#include <glm/glm.hpp>
#include <lava/core/ray.hpp>
#include <lava/core/transform.hpp>
#include <lava/core/vector-view.hpp>

namespace lava::dike {
    class PhysicsEngine;
}

namespace lava::dike {
    /**
     * Manages a dynamic or static rigid body living in the physics world.
     *
     * A rigid body is a compound of shapes.
     */
    class RigidBody final {
    public:
        RigidBody(PhysicsEngine& engine);
        ~RigidBody();

        // Shapes
        void clearShapes();
        void addBoxShape(const glm::vec3& offset = {0.f, 0.f, 0.f}, const glm::vec3& dimensions = {1.f, 1.f, 1.f});
        void addSphereShape(const glm::vec3& offset = {0.f, 0.f, 0.f}, float diameter = 1.f);
        void addInfinitePlaneShape(const glm::vec3& offset = {0.f, 0.f, 0.f}, const glm::vec3& normal = {0.f, 0.f, 1.f});

        // @note The indices have to be tighly packed, therefore we use a vector directly.
        void addMeshShape(const glm::mat4& localTransform, VectorView<glm::vec3> vertices, const std::vector<uint16_t>& indices);

        // Physics world
        bool enabled() const;
        void enabled(bool enabled);

        bool dynamic() const;
        void dynamic(bool dynamic);

        // Whether the transform changed during last update.
        bool transformChanged() const;

        const lava::Transform& transform() const;
        void transform(const lava::Transform& transform);

        // Helpers
        float distanceFrom(const Ray& ray, float maxDistance = 1000.f) const;

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
