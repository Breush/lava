#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/glm.hpp>
#include <string>

namespace lava::sill {
    class BoxColliderComponent final : public IComponent {
    public:
        /// Creates a cube box of size 1.
        BoxColliderComponent(GameEntity& entity);
        /// Creates a cube box of specified size.
        BoxColliderComponent(GameEntity& entity, float cubeSize);
        /// Creates a box of specified size.
        BoxColliderComponent(GameEntity& entity, const glm::vec3& dimensions);

        ~BoxColliderComponent();

        void dimensions(const glm::vec3& dimensions);

        // Box rigid body
        bool enabled() const;
        void enabled(bool enabled);

        /**
         * @name Debug
         * ```
         */
        /// @{
        /// A wireframe object that show the collider's shape.
        void debugEnabled(bool debugEnabled);
        /// @}

        // IComponent
        static std::string hrid() { return "box-collider"; }
        void update(float dt) final;

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
