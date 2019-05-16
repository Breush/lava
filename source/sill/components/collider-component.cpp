#include <lava/sill/components/collider-component.hpp>

#include "./collider-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(ColliderComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(ColliderComponent, void, update, float, dt);

// Shapes
$pimpl_method(ColliderComponent, void, clearShapes);
$pimpl_method(ColliderComponent, void, addBoxShape, const glm::vec3&, offset, const glm::vec3&, dimensions);
$pimpl_method(ColliderComponent, void, addSphereShape, const glm::vec3&, offset, float, diameter);
$pimpl_method(ColliderComponent, void, addInfinitePlaneShape, const glm::vec3&, offset, const glm::vec3&, normal);

void ColliderComponent::addBoxShape(const glm::vec3& offset, float cubeSize)
{
    m_impl->addBoxShape(offset, {cubeSize, cubeSize, cubeSize});
}

// Debug
$pimpl_method(ColliderComponent, void, debugEnabled, bool, debugEnabled);
