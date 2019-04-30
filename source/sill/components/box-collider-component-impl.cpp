#include "./box-collider-component-impl.hpp"

#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>

using namespace lava::sill;

BoxColliderComponent::Impl::Impl(GameEntity& entity, const glm::vec3& dimensions)
    : ComponentImpl(entity)
    , m_physicsEngine(m_entity.engine().physicsEngine())
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    m_rigidBody = &m_physicsEngine.make<dike::BoxRigidBody>(dimensions);

    m_transformComponent.onTransformChanged([this]() { onTransformChanged(); }, ~TransformComponent::ChangeReasonFlag::Physics);

    // Init correctly on first creation
    onTransformChanged();
}

BoxColliderComponent::Impl::~Impl()
{
    // @fixme Remove from dike
}

void BoxColliderComponent::Impl::dimensions(const glm::vec3& dimensions)
{
    m_rigidBody->dimensions(dimensions);
}

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
