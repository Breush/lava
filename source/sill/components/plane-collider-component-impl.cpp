#include "./plane-collider-component-impl.hpp"

#include "../game-engine-impl.hpp"
#include "../game-entity-impl.hpp"

using namespace lava::sill;

// @fixme Have a way to handle multiple colliders for the same entity.
// We might want a generic "ColliderComponent" with makers for spheres, planes and such.

PlaneColliderComponent::Impl::Impl(GameEntity& entity, const glm::vec3& normal)
    : ComponentImpl(entity)
    , m_physicsEngine(m_entity.engine().physicsEngine())
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    m_staticRigidBody = &m_physicsEngine.make<dike::PlaneStaticRigidBody>(normal);

    // @fixme Clarify API
    // We can't just get offset and normal from TransformComponent/normal/position
    // as the collider might be offseted
}

PlaneColliderComponent::Impl::~Impl()
{
}
