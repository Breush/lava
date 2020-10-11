#pragma once

#include <lava/sill/components/i-component.hpp>

#include <functional>
#include <string>

namespace lava::sill {
    class BehaviorComponent final : public IComponent {
        using UpdateCallback = std::function<void(float)>;

    public:
        BehaviorComponent(Entity& entity);

        // IComponent
        static std::string hrid() { return "behavior"; }
        void update(float dt) final;

        // Interface
        void onUpdate(UpdateCallback&& updateCallback);

    private:
        UpdateCallback m_updateCallback;
    };
}
