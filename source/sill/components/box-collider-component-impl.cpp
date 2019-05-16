#include "./box-collider-component-impl.hpp"

#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>
#include <lava/sill/makers/box-mesh.hpp>

using namespace lava::sill;

BoxColliderComponent::Impl::Impl(GameEntity& entity, const glm::vec3& dimensions)
    : ComponentImpl(entity)
    , m_physicsEngine(m_entity.engine().physicsEngine())
    , m_transformComponent(entity.ensure<TransformComponent>())
    , m_dimensions(dimensions)
{
    m_rigidBody = &m_physicsEngine.make<dike::BoxRigidBody>(dimensions);

    m_transformComponent.onTransformChanged([this]() { onTransformChanged(); });
    m_transformComponent.onTransformChanged([this]() { onNonPhysicsTransformChanged(); },
                                            ~TransformComponent::ChangeReasonFlag::Physics);

    // Init correctly on first creation
    onTransformChanged();
}

BoxColliderComponent::Impl::~Impl()
{
    m_physicsEngine.remove(*m_rigidBody);

    if (m_debugEnabled) {
        debugEnabled(false);
    }
}

void BoxColliderComponent::Impl::dimensions(const glm::vec3& dimensions)
{
    m_dimensions = dimensions;
    m_rigidBody->dimensions(dimensions);

    // Refresh debug shape if needed.
    if (m_debugEnabled) {
        debugEnabled(false);
        debugEnabled(true);
    }
}

//----- Debug

void BoxColliderComponent::Impl::debugEnabled(bool debugEnabled)
{
    if (m_debugEnabled == debugEnabled) return;
    m_debugEnabled = debugEnabled;

    if (debugEnabled) {
        m_debugEntity = &m_entity.engine().make<GameEntity>();
        auto& debugMeshComponent = m_debugEntity->make<MeshComponent>();
        makers::boxMeshMaker(m_dimensions)(debugMeshComponent);
        debugMeshComponent.wireframed(true);
    }
    else {
        m_entity.engine().remove(*m_debugEntity);
        m_debugEntity = nullptr;
    }
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
    if (m_debugEnabled) {
        const auto& worldTransform = m_transformComponent.worldTransform();
        m_debugEntity->get<TransformComponent>().worldTransform(worldTransform);
    }
}

void BoxColliderComponent::Impl::onNonPhysicsTransformChanged()
{
    const auto& worldTransform = m_transformComponent.worldTransform();
    m_rigidBody->transform(worldTransform);
}
