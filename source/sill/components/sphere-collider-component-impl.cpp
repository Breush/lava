#include "./sphere-collider-component-impl.hpp"

#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>

using namespace lava::sill;

SphereColliderComponent::Impl::Impl(GameEntity& entity, float diameter)
    : ComponentImpl(entity)
    , m_physicsEngine(m_entity.engine().physicsEngine())
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    m_rigidBody = &m_physicsEngine.make<dike::SphereRigidBody>(diameter);

    m_transformComponent.onTransformChanged([this]() { onTransformChanged(); }, ~TransformComponent::ChangeReasonFlag::Physics);

    // Init correctly on first creation
    onTransformChanged();
}

SphereColliderComponent::Impl::~Impl()
{
    m_physicsEngine.remove(*m_rigidBody);
}

//----- IComponent

void SphereColliderComponent::Impl::update(float /* dt */)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_transformComponent.worldTransform(m_rigidBody->transform(), TransformComponent::ChangeReasonFlag::Physics);
}

//----- Callbacks

void SphereColliderComponent::Impl::onTransformChanged()
{
    m_rigidBody->transform(m_transformComponent.worldTransform());
}
