#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/glm.hpp>
#include <string>

namespace lava::sill {
    class TransformComponent;
    class PhysicsComponent;
    class MeshNode;
}

namespace lava::sill {
    /**
     * Handles complex shapes that make either colliders.
     *
     * An entity will be physics-animated only if it has both
     * PhysicsComponent and a collider.
     */
    class ColliderComponent final : public IComponent {
    public:
        struct BoxShape {
            glm::vec3 offset;
            glm::vec3 extent;
            GameEntity* debugEntity = nullptr;
        };

        struct SphereShape {
            glm::vec3 offset;
            float diameter;
        };

        struct InfinitePlaneShape {
            glm::vec3 offset;
            glm::vec3 normal;
        };

    public:
        ColliderComponent(GameEntity& entity);
        ~ColliderComponent();

        // IComponent
        static std::string hrid() { return "collider"; }

        /**
         * @name Shapes
         */
        /// @{
        /// Clears all previous shapes.
        void clearShapes();
        /// Creates a cube box of specified size.
        void addBoxShape(const glm::vec3& offset, float cubeSize) { addBoxShape(offset, glm::vec3{cubeSize, cubeSize, cubeSize}); }
        /// Creates a box of specified size.
        void addBoxShape(const glm::vec3& offset = {0.f, 0.f, 0.f}, const glm::vec3& extent = {1.f, 1.f, 1.f});
        /// Creates a sphere of specified size.
        void addSphereShape(const glm::vec3& offset, float diameter);
        /// Creates an non-dynamic infinte plane.
        void addInfinitePlaneShape(const glm::vec3& offset = {0.f, 0.f, 0.f}, const glm::vec3& normal = {0.f, 0.f, 1.f});
        /// Creates a collider from the current mesh component primitives.
        void addMeshShape();
        /// Add a collider from any mesh node, will also add all children.
        void addMeshNodeShape(const MeshNode& node);

        const std::vector<BoxShape>& boxShapes() const { return m_boxShapes; }
        const std::vector<SphereShape>& sphereShapes() const { return m_sphereShapes; }
        const std::vector<InfinitePlaneShape>& infinitePlaneShapes() const { return m_infinitePlaneShapes; }
        /// @}

        /**
         * @name Debug
         */
        /// @{
        /// A wireframe object that show the collider's shapes.
        /// @note Only works for box shapes currently.
        void debugEnabled(bool debugEnabled);
        /// @}

    protected:
        void onWorldTransformChanged();

    private:
        // References
        TransformComponent& m_transformComponent;
        PhysicsComponent& m_physicsComponent;

        // Resources
        std::vector<BoxShape> m_boxShapes;
        std::vector<SphereShape> m_sphereShapes;
        std::vector<InfinitePlaneShape> m_infinitePlaneShapes;

        // Debug
        bool m_debugEnabled = false;
    };
}
