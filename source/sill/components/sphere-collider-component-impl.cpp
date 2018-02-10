#include "./sphere-collider-component-impl.hpp"

#include "../game-engine-impl.hpp"
#include "../game-entity-impl.hpp"

using namespace lava::sill;

SphereColliderComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
    , m_physicsEngine(m_entity.engine().physicsEngine())
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    // @fixme Get radius from somewhere
    m_rigidBody = &m_physicsEngine.make<dike::SphereRigidBody>(0.1f);

    m_transformComponent.onPositionChanged([this](const glm::vec3& position) { onPositionChanged(position); },
                                           ~TransformComponent::ChangeReasonFlag::Physics);

    // Init correctly on first creation
    onPositionChanged(m_transformComponent.position());
}

SphereColliderComponent::Impl::~Impl()
{
}

//----- IComponent

void SphereColliderComponent::Impl::update()
{
    m_transformComponent.position(m_rigidBody->position(), TransformComponent::ChangeReasonFlag::Physics);
}

//----- Callbacks

void SphereColliderComponent::Impl::onPositionChanged(const glm::vec3& position)
{
    auto delta = position - m_rigidBody->position();
    m_rigidBody->positionAdd(delta);
}
