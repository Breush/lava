#include <lava/sill/components/behavior-component.hpp>

using namespace lava::sill;

BehaviorComponent::BehaviorComponent(Entity& entity)
    : IComponent(entity)
{
}

// ----- IComponent

void BehaviorComponent::update(float dt)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    if (m_updateCallback) {
        m_updateCallback(dt);
    }
}

// ----- Callbacks

void BehaviorComponent::onUpdate(UpdateCallback&& updateCallback)
{
    m_updateCallback = std::move(updateCallback);
}
