#pragma once

#include <lava/sill/components/box-collider-component.hpp>
#include <lava/sill/components/transform-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class BoxColliderComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity, const glm::vec3& dimensions);
        ~Impl();

        void dimensions(const glm::vec3& dimensions);

        // Box rigid body
        bool enabled() const { return m_rigidBody->enabled(); }
        void enabled(bool enabled) { m_rigidBody->enabled(enabled); }

        // IComponent
        void update(float dt) override final;

    protected:
        /// Callbacks
        void onTransformChanged();

    private:
        // References
        dike::PhysicsEngine& m_physicsEngine;
        TransformComponent& m_transformComponent;

        dike::BoxRigidBody* m_rigidBody = nullptr;
    };
}
