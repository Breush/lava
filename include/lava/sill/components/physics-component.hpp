#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/glm.hpp>
#include <lava/core/ray.hpp>
#include <string>

namespace lava::dike {
    class RigidBody;
}

namespace lava::sill {
    class TransformComponent;
}

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

        dike::RigidBody& rigidBody() { return *m_rigidBody; }

        // IComponent
        static std::string hrid() { return "physics"; }
        void update(float dt) final;

        /// Controls whether the entity should exists in the physics world.
        bool enabled() const;
        void enabled(bool enabled);

        /// Controls whether the entity can move (defaults: true).
        bool dynamic() const;
        void dynamic(bool dynamic);

        /**
         * @name Helpers
         */
        /// @{
        float distanceFrom(const Ray& ray) const;
        /// @}

    private:
        // References
        dike::PhysicsEngine& m_physicsEngine;
        TransformComponent& m_transformComponent;

        dike::RigidBody* m_rigidBody = nullptr;
    };
}
