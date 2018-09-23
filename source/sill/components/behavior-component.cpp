#include <lava/sill/components/behavior-component.hpp>

#include "./behavior-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(BehaviorComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(BehaviorComponent, void, update, float, dt);

// Callbacks
$pimpl_method(BehaviorComponent, void, onUpdate, UpdateCallback, updateCallback);
