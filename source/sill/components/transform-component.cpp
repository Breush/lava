#include <lava/sill/components/transform-component.hpp>

#include <lava/core/macros.hpp>

#include "./transform-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(TransformComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(TransformComponent, void, update, float, dt);

// Local transform
$pimpl_method_const(TransformComponent, glm::vec3, translation);
$pimpl_method(TransformComponent, void, translation, const glm::vec3&, translation, ChangeReasonFlag, changeReasonFlag);
$pimpl_method(TransformComponent, void, translate, const glm::vec3&, delta, ChangeReasonFlag, changeReasonFlag);

$pimpl_method(TransformComponent, void, rotate, const glm::vec3&, axis, float, angle, ChangeReasonFlag, changeReasonFlag);

$pimpl_method_const(TransformComponent, glm::vec3, scaling);
$pimpl_method(TransformComponent, void, scaling, const glm::vec3&, scaling, ChangeReasonFlag, changeReasonFlag);
$pimpl_method(TransformComponent, void, scale, const glm::vec3&, factors, ChangeReasonFlag, changeReasonFlag);
$pimpl_method(TransformComponent, void, scale, float, factor, ChangeReasonFlag, changeReasonFlag);

// World transform
$pimpl_method_const(TransformComponent, const glm::mat4&, worldTransform);

// Callbacks
$pimpl_method(TransformComponent, void, onTransformChanged, std::function<void()>, transformChangedCallback, ChangeReasonFlags,
              changeReasonFlags);
