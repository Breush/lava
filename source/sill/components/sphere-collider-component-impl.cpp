#include "./sphere-collider-component-impl.hpp"

#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>

using namespace lava::sill;

SphereColliderComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
    , m_physicsEngine(m_entity.engine().physicsEngine())
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    // @fixme Get radius from somewhere
    m_rigidBody = &m_physicsEngine.make<dike::SphereRigidBody>(0.1f);

    m_transformComponent.onTransformChanged([this]() { onTransformChanged(); }, ~TransformComponent::ChangeReasonFlag::Physics);

    // Init correctly on first creation
    onTransformChanged();
}

SphereColliderComponent::Impl::~Impl() {}

//----- IComponent

void SphereColliderComponent::Impl::update()
{
    m_transformComponent.translation(m_rigidBody->translation(), TransformComponent::ChangeReasonFlag::Physics);
}

//----- Callbacks

void SphereColliderComponent::Impl::onTransformChanged()
{
    // @todo Should be able to set transform here directly!
    auto delta = m_transformComponent.translation() - m_rigidBody->translation();
    m_rigidBody->translate(delta);
}
