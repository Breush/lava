#pragma once

#include <lava/sill/components/behavior-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class BehaviorComponent::Impl : public ComponentImpl {
    public:
        Impl(Entity& entity);

        // IComponent
        void update(float dt);

        // BehaviorComponent
        void onUpdate(UpdateCallback&& updateCallback);

    private:
        UpdateCallback m_updateCallback;
    };
}
