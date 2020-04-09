#pragma once

#include <lava/sill/components/physics-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class TransformComponent;
}

namespace lava::sill {
    class PhysicsComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);
        ~Impl();

        // IComponent
        void update(float dt);

        dike::RigidBody& dike() { return *m_rigidBody; }

        // PhysicsComponent
        bool enabled() const { return m_rigidBody->enabled(); }
        void enabled(bool enabled) { m_rigidBody->enabled(enabled); }

        bool dynamic() const { return m_rigidBody->dynamic(); }
        void dynamic(bool dynamic) { m_rigidBody->dynamic(dynamic); }

    protected:
        /// Callbacks
        void onNonPhysicsWorldTransformChanged();

    private:
        // References
        dike::PhysicsEngine& m_physicsEngine;
        TransformComponent& m_transformComponent;

        dike::RigidBody* m_rigidBody = nullptr;
    };
}
