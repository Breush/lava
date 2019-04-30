#include "./plane-collider-component-impl.hpp"

#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>

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
    // We can't just get offset and normal from TransformComponent/normal/translation
    // as the collider might be offseted
}

PlaneColliderComponent::Impl::~Impl()
{
    m_physicsEngine.remove(*m_staticRigidBody);
}
