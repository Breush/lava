#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/glm.hpp>
#include <string>

namespace lava::sill {
    class SphereColliderComponent final : public IComponent {
    public:
        /// Creates a sphere of diameter 1.
        SphereColliderComponent(GameEntity& entity);
        /// Creates a sphere of specified diameter.
        SphereColliderComponent(GameEntity& entity, float diameter);
        ~SphereColliderComponent();

        // IComponent
        static std::string hrid() { return "sphere-collider"; }
        void update(float dt) override final;

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
