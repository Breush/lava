#pragma once

#include <lava/sill/components/i-component.hpp>

#include <functional>
#include <string>

namespace lava::sill {
    class BehaviorComponent final : public IComponent {
        using UpdateCallback = std::function<void(float)>;

    public:
        BehaviorComponent(GameEntity& entity);
        ~BehaviorComponent();

        // IComponent
        static std::string hrid() { return "behavior"; }
        void update(float dt) final;

        // Interface
        void onUpdate(UpdateCallback updateCallback);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
