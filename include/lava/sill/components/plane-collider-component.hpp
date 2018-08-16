#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/glm.hpp>
#include <string>

namespace lava::sill {
    class PlaneColliderComponent final : public IComponent {
    public:
        PlaneColliderComponent(GameEntity& entity, const glm::vec3& normal = {0.f, 0.f, 1.f});
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
