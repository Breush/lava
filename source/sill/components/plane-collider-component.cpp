#include <lava/sill/components/plane-collider-component.hpp>

#include "./plane-collider-component-impl.hpp"

using namespace lava::sill;

PlaneColliderComponent::PlaneColliderComponent(GameEntity& entity, const glm::vec3& normal)
    : IComponent(entity)
{
    m_impl = new Impl(entity, normal);
}

PlaneColliderComponent::~PlaneColliderComponent()
{
    delete m_impl;
}

// IComponent
$pimpl_method(PlaneColliderComponent, void, update, float, dt);
