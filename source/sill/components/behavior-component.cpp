#include <lava/sill/components/behavior-component.hpp>

#include "./behavior-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(BehaviorComponent, IComponent, Entity&, entity);

// IComponent
$pimpl_method(BehaviorComponent, void, update, float, dt);

// Callbacks
void BehaviorComponent::onUpdate(UpdateCallback&& updateCallback)
{
    m_impl->onUpdate(std::move(updateCallback));
}
