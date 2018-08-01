#include <lava/sill/components/transform-component.hpp>

#include <lava/core/macros.hpp>

#include "./transform-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(TransformComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(TransformComponent, void, update);

$pimpl_method(TransformComponent, void, translate, const glm::vec3&, delta, ChangeReasonFlag, changeReasonFlag);
$pimpl_method_const(TransformComponent, glm::vec3, translation);
$pimpl_method(TransformComponent, void, translation, const glm::vec3&, translation, ChangeReasonFlag, changeReasonFlag);
$pimpl_method_const(TransformComponent, const glm::mat4&, worldTransform);

// Callbacks
$pimpl_method(TransformComponent, void, onTranslationChanged, std::function<void(const glm::vec3&)>, positionChangedCallback,
              ChangeReasonFlags, changeReasonFlags);
