#pragma once

#include <lava/dike/rigid-bodies/sphere-rigid-body.hpp>
#include <lava/sill/components/sphere-collider-component.hpp>
#include <lava/sill/components/transform-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class SphereColliderComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);
        ~Impl();

        // IComponent
        void update() override final;

    protected:
        /// Callbacks
        void onTranslationChanged(const glm::vec3& translation);

    private:
        // References
        dike::PhysicsEngine& m_physicsEngine;
        TransformComponent& m_transformComponent;

        dike::SphereRigidBody* m_rigidBody = nullptr;
    };
}
