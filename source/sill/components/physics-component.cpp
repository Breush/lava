#include <lava/sill/components/physics-component.hpp>

#include "./physics-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(PhysicsComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(PhysicsComponent, void, update, float, dt);

// Physics world
$pimpl_method_const(PhysicsComponent, bool, enabled);
$pimpl_method(PhysicsComponent, void, enabled, bool, enabled);
$pimpl_method_const(PhysicsComponent, bool, dynamic);
$pimpl_method(PhysicsComponent, void, dynamic, bool, dynamic);
