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

    m_transformComponent.onTranslationChanged([this](const glm::vec3& translation) { onTranslationChanged(translation); },
                                              ~TransformComponent::ChangeReasonFlag::Physics);

    // Init correctly on first creation
    onTranslationChanged(m_transformComponent.translation());
}

SphereColliderComponent::Impl::~Impl() {}

//----- IComponent

void SphereColliderComponent::Impl::update()
{
    m_transformComponent.translation(m_rigidBody->translation(), TransformComponent::ChangeReasonFlag::Physics);
}

//----- Callbacks

void SphereColliderComponent::Impl::onTranslationChanged(const glm::vec3& translation)
{
    auto delta = translation - m_rigidBody->translation();
    m_rigidBody->translate(delta);
}
