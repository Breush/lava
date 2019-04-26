#include <lava/sill/components/box-collider-component.hpp>

#include "./box-collider-component-impl.hpp"

using namespace lava::sill;

BoxColliderComponent::BoxColliderComponent(GameEntity& entity)
    : BoxColliderComponent(entity, glm::vec3{1.f, 1.f, 1.f})
{
}

BoxColliderComponent::BoxColliderComponent(GameEntity& entity, float cubeSize)
    : BoxColliderComponent(entity, glm::vec3{cubeSize, cubeSize, cubeSize})
{
}

BoxColliderComponent::BoxColliderComponent(GameEntity& entity, const glm::vec3& dimensions)
    : IComponent(entity)
{
    m_impl = new BoxColliderComponent::Impl(entity, dimensions);
}

BoxColliderComponent::~BoxColliderComponent()
{
    delete m_impl;
}

// Box rigid body
$pimpl_method_const(BoxColliderComponent, bool, enabled);
$pimpl_method(BoxColliderComponent, void, enabled, bool, enabled);

// IComponent
$pimpl_method(BoxColliderComponent, void, update, float, dt);
