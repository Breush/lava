#include "./physics-component-impl.hpp"

#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>

using namespace lava::sill;

PhysicsComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
    , m_physicsEngine(m_entity.engine().physicsEngine())
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    m_rigidBody = &m_physicsEngine.make<dike::RigidBody>();

    m_transformComponent.onTransformChanged([this]() { onNonPhysicsTransformChanged(); },
                                            ~TransformComponent::ChangeReasonFlag::Physics);

    // Init correctly on first creation
    onNonPhysicsTransformChanged();
}

PhysicsComponent::Impl::~Impl()
{
    m_physicsEngine.remove(*m_rigidBody);
}

//----- IComponent

void PhysicsComponent::Impl::update(float /* dt */)
{
    // @fixme Have a flag in dike, so that we don't update uselessly

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_transformComponent.worldTransform(m_rigidBody->transform(), TransformComponent::ChangeReasonFlag::Physics);
}

//----- Callbacks

void PhysicsComponent::Impl::onNonPhysicsTransformChanged()
{
    const auto& worldTransform = m_transformComponent.worldTransform();
    m_rigidBody->transform(worldTransform);
}
