#include <lava/sill/components/animation-component.hpp>

#include "./animation-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(AnimationComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(AnimationComponent, void, update, float, dt);

// Callbacks
$pimpl_method(AnimationComponent, void, start, AnimationFlags, flags, float, time);
$pimpl_method(AnimationComponent, void, stop, AnimationFlags, flags);
$pimpl_method(AnimationComponent, void, target, AnimationFlag, flag, const glm::mat4&, target);
