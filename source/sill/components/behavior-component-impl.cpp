#include "./behavior-component-impl.hpp"

using namespace lava::sill;

BehaviorComponent::Impl::Impl(Entity& entity)
    : ComponentImpl(entity)
{
}

// IComponent
void BehaviorComponent::Impl::update(float dt)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    if (m_updateCallback) {
        m_updateCallback(dt);
    }
}

// BehaviorComponent
void BehaviorComponent::Impl::onUpdate(UpdateCallback&& updateCallback)
{
    m_updateCallback = std::move(updateCallback);
}
