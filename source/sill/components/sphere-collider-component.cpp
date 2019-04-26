#include <lava/sill/components/sphere-collider-component.hpp>

#include "./sphere-collider-component-impl.hpp"

using namespace lava::sill;

SphereColliderComponent::SphereColliderComponent(GameEntity& entity)
    : SphereColliderComponent(entity, 1.f)
{
}

SphereColliderComponent::SphereColliderComponent(GameEntity& entity, float diameter)
    : IComponent(entity)
{
    m_impl = new SphereColliderComponent::Impl(entity, diameter);
}

SphereColliderComponent::~SphereColliderComponent()
{
    delete m_impl;
}

// Sphere rigid body
$pimpl_method_const(SphereColliderComponent, bool, enabled);
$pimpl_method(SphereColliderComponent, void, enabled, bool, enabled);

// IComponent
$pimpl_method(SphereColliderComponent, void, update, float, dt);
