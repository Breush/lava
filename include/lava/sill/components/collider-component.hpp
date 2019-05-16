#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/glm.hpp>
#include <string>

namespace lava::sill {
    /**
     * Handles complex shapes that make either colliders.
     *
     * An entity will be physics-animated only if it has both
     * PhysicsComponent and a collider.
     */
    class ColliderComponent final : public IComponent {
    public:
        ColliderComponent(GameEntity& entity);
        ~ColliderComponent();

        // IComponent
        static std::string hrid() { return "collider"; }
        void update(float dt) final;

        /**
         * @name Shapes
         */
        /// @{
        /// Clears all previous shapes.
        void clearShapes();
        /// Creates a cube box of specified size.
        void addBoxShape(const glm::vec3& offset, float cubeSize);
        /// Creates a box of specified size.
        void addBoxShape(const glm::vec3& offset = {0.f, 0.f, 0.f}, const glm::vec3& dimensions = {1.f, 1.f, 1.f});
        /// Creates a sphere of specified size.
        void addSphereShape(const glm::vec3& offset, float diameter);
        /// Creates an non-dynamic infinte plane.
        void addInfinitePlaneShape(const glm::vec3& offset = {0.f, 0.f, 0.f}, const glm::vec3& normal = {0.f, 0.f, 1.f});
        /// @}

        /**
         * @name Debug
         */
        /// @{
        /// A wireframe object that show the collider's shapes.
        void debugEnabled(bool debugEnabled);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
