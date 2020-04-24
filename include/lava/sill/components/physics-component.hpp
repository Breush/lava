#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/glm.hpp>
#include <string>

namespace lava::sill {
    /**
     * Handles dynamic and static physics.
     *
     * An entity needs a ColliderComponent too to be
     * able to be used within the physics world.
     */
    class PhysicsComponent final : public IComponent {
    public:
        PhysicsComponent(GameEntity& entity);
        ~PhysicsComponent();

        // IComponent
        static std::string hrid() { return "physics"; }
        void update(float dt) final;

        /// Controls whether the entity should exists in the physics world.
        bool enabled() const;
        void enabled(bool enabled);

        /// Controls whether the entity can move (defaults: true).
        bool dynamic() const;
        void dynamic(bool dynamic);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
