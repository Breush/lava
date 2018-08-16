#include <lava/sill/components/sphere-collider-component.hpp>

#include <lava/core/macros.hpp>

#include "./sphere-collider-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(SphereColliderComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(SphereColliderComponent, void, update, float, dt);
