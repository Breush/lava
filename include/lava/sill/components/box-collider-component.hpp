#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/glm.hpp>
#include <string>

namespace lava::sill {
    class BoxColliderComponent final : public IComponent {
    public:
        BoxColliderComponent(GameEntity& entity);
        ~BoxColliderComponent();

        // IComponent
        static std::string hrid() { return "box-collider"; }
        void update(float dt) final;

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
