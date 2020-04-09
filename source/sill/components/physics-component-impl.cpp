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

    m_transformComponent.onWorldTransformChanged([this]() { onNonPhysicsWorldTransformChanged(); },
                                                 ~TransformComponent::ChangeReasonFlag::Physics);

    // Init correctly on first creation
    onNonPhysicsWorldTransformChanged();
}

PhysicsComponent::Impl::~Impl()
{
    m_rigidBody->clearShapes();
    m_physicsEngine.remove(*m_rigidBody);
}

//----- IComponent

void PhysicsComponent::Impl::update(float /* dt */)
{
    if (!m_rigidBody->transformChanged()) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_transformComponent.worldTransform(m_rigidBody->transform(), TransformComponent::ChangeReasonFlag::Physics);
}

//----- Callbacks

void PhysicsComponent::Impl::onNonPhysicsWorldTransformChanged()
{
    m_rigidBody->transform(m_transformComponent.worldTransform());
}
