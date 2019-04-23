#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/glm.hpp>
#include <string>

namespace lava::sill {
    class PlaneColliderComponent final : public IComponent {
    public:
        /// Creates a plane with normal {0.f, 0.f, 1.f}.
        PlaneColliderComponent(GameEntity& entity);
        /// Creates a plane with specified normal.
        PlaneColliderComponent(GameEntity& entity, const glm::vec3& normal);
        ~PlaneColliderComponent();

        // IComponent
        static std::string hrid() { return "plane-collider"; }
        void update(float dt) override final;

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
