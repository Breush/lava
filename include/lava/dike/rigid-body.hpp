#pragma once

#include <glm/glm.hpp>

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

        // Physics world
        bool enabled() const;
        void enabled(bool enabled);

        bool dynamic() const;
        void dynamic(bool dynamic);

        const glm::mat4& transform() const;
        void transform(const glm::mat4& transform);

    private:
        class Impl;
        Impl* m_impl = nullptr;
    };
}
