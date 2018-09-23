#pragma once

#include <lava/sill/components/sphere-collider-component.hpp>
#include <lava/sill/components/transform-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class SphereColliderComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);
        ~Impl();

        // IComponent
        void update(float dt) override final;

    protected:
        /// Callbacks
        void onTransformChanged();

    private:
        // References
        dike::PhysicsEngine& m_physicsEngine;
        TransformComponent& m_transformComponent;

        dike::SphereRigidBody* m_rigidBody = nullptr;
    };
}
