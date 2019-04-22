#include <lava/sill/components/box-collider-component.hpp>

#include "./box-collider-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(BoxColliderComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(BoxColliderComponent, void, update, float, dt);
