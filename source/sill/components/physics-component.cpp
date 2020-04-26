#include <lava/sill/components/physics-component.hpp>

#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>

using namespace lava::sill;

PhysicsComponent::PhysicsComponent(GameEntity& entity)
    : IComponent(entity)
    , m_physicsEngine(entity.engine().physicsEngine())
    , m_transformComponent(entity.ensure<TransformComponent>())
{
    m_rigidBody = &m_physicsEngine.make<dike::RigidBody>();

    m_transformComponent.onWorldTransformChanged([this]() { m_rigidBody->transform(m_transformComponent.worldTransform()); },
                                                 ~TransformComponent::ChangeReasonFlag::Physics);

    // Init correctly on first creation
    m_rigidBody->transform(m_transformComponent.worldTransform());
}

PhysicsComponent::~PhysicsComponent()
{
    m_physicsEngine.remove(*m_rigidBody);
}

// ----- IComponent

void PhysicsComponent::update(float /* dt */)
{
    if (!m_rigidBody->transformChanged()) return;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_transformComponent.worldTransform(m_rigidBody->transform(), TransformComponent::ChangeReasonFlag::Physics);
}

// ----- World

bool PhysicsComponent::enabled() const
{
    return m_rigidBody->enabled();
}

void PhysicsComponent::enabled(bool enabled)
{
    m_rigidBody->enabled(enabled);
}

bool PhysicsComponent::dynamic() const
{
    return m_rigidBody->dynamic();
}

void PhysicsComponent::dynamic(bool dynamic)
{
    m_rigidBody->dynamic(dynamic);
}

// ----- Helpers

float PhysicsComponent::distanceFrom(const Ray& ray) const
{
    return m_rigidBody->distanceFrom(ray);
}
