#include <lava/sill/components/transform-component.hpp>

#include <lava/chamber/macros.hpp>

#include "./transform-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(TransformComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(TransformComponent, void, update);
$pimpl_method(TransformComponent, void, postUpdate);

$pimpl_method_const(TransformComponent, bool, changed);
$pimpl_method(TransformComponent, void, positionAdd, const glm::vec3&, delta, ChangeReasonFlag, changeReasonFlag);
$pimpl_method_const(TransformComponent, glm::vec3, position);
$pimpl_method(TransformComponent, void, position, const glm::vec3&, position, ChangeReasonFlag, changeReasonFlag);
$pimpl_method_const(TransformComponent, const glm::mat4&, worldTransform);

// Callbacks
$pimpl_method(TransformComponent, void, onPositionChanged, std::function<void(const glm::vec3&)>, positionChangedCallback,
              ChangeReasonFlags, changeReasonFlags);
