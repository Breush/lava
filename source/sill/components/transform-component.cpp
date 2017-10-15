#include <lava/sill/components/transform-component.hpp>

#include <lava/chamber/macros.hpp>

#include "./transform-component-impl.hpp"

using namespace lava::sill;

$pimpl_class(TransformComponent, GameEntity&, entity);

// IComponent
$pimpl_method(TransformComponent, void, update);
$pimpl_method(TransformComponent, void, postUpdate);

$pimpl_method_const(TransformComponent, bool, changed);
$pimpl_method(TransformComponent, void, positionAdd, const glm::vec3&, delta);
$pimpl_method_const(TransformComponent, const glm::mat4&, worldTransform);
