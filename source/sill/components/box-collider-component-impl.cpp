#include "./box-collider-component-impl.hpp"

#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>

using namespace lava::sill;

BoxColliderComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
    , m_physicsEngine(m_entity.engine().physicsEngine())
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    // @fixme Get dimensions from somewhere
    m_rigidBody = &m_physicsEngine.make<dike::BoxRigidBody>(glm::vec3{0.1f, 0.1f, 0.1f});

    m_transformComponent.onTransformChanged([this]() { onTransformChanged(); }, ~TransformComponent::ChangeReasonFlag::Physics);

    // Init correctly on first creation
    onTransformChanged();
}

BoxColliderComponent::Impl::~Impl() {}

//----- IComponent

void BoxColliderComponent::Impl::update(float /* dt */)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_transformComponent.worldTransform(m_rigidBody->transform(), TransformComponent::ChangeReasonFlag::Physics);
}

//----- Callbacks

void BoxColliderComponent::Impl::onTransformChanged()
{
    m_rigidBody->transform(m_transformComponent.worldTransform());
}
