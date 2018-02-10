#pragma once

#include <lava/dike/static-rigid-bodies/plane-static-rigid-body.hpp>
#include <lava/sill/components/plane-collider-component.hpp>
#include <lava/sill/components/transform-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class PlaneColliderComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity, const glm::vec3& normal);
        ~Impl();

        // IComponent
        void update() override final {}
        void postUpdate() override final {}

    private:
        // References
        dike::PhysicsEngine& m_physicsEngine;
        TransformComponent& m_transformComponent;

        dike::PlaneStaticRigidBody* m_staticRigidBody = nullptr;
    };
}
