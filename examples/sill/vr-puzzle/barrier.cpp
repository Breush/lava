#include "./barrier.hpp"

#include <iostream>

#include "./game-state.hpp"

using namespace lava;

Barrier::Barrier(GameState& gameState)
    : m_gameState(gameState)
{
    m_entity = &gameState.engine->make<sill::GameEntity>("barrier");
    m_entity->make<sill::TransformComponent>();
    m_entity->make<sill::MeshComponent>();
    m_entity->make<sill::AnimationComponent>();
    m_entity->make<sill::ColliderComponent>();

    auto& meshComponent = m_entity->get<sill::MeshComponent>();
    sill::makers::CylinderMeshOptions options = {.doubleSided = true, .offset = 1.5f};
    sill::makers::cylinderMeshMaker(16u, 1.f, 3.f, options)(meshComponent);
    meshComponent.primitive(0u, 0u).category(RenderCategory::Translucent);
    meshComponent.primitive(0u, 0u).shadowsCastable(false);

    auto& barrierMaterial = gameState.engine->scene().make<magma::Material>("barrier");
    meshComponent.primitive(0u, 0u).material(barrierMaterial);
}

Barrier::~Barrier()
{
    m_gameState.engine->remove(*m_entity);
}

void Barrier::diameter(float diameter)
{
    m_diameter = diameter;
    transform().scaling(glm::vec3{diameter, diameter, 1.f});
}

void Barrier::powered(bool powered)
{
    if (m_powered == powered) return;
    m_powered = powered;

    animation().start(sill::AnimationFlag::MaterialUniform, material(), "poweredRatio", 0.5f);
    animation().target(sill::AnimationFlag::MaterialUniform, material(), "poweredRatio", (powered) ? 1.f : 0.f);
}

// -----

Barrier* findBarrierByName(GameState& gameState, const std::string& name)
{
    for (const auto& barrier : gameState.level.barriers) {
        if (barrier->name() == name) {
            return barrier.get();
        }
    }

    return nullptr;
}

Barrier* findBarrier(GameState& gameState, const sill::GameEntity& entity)
{
    for (const auto& barrier : gameState.level.barriers) {
        if (&barrier->entity() == &entity) {
            return barrier.get();
        }
    }

    return nullptr;
}

uint32_t findBarrierIndex(GameState& gameState, const sill::GameEntity& entity)
{
    for (uint32_t i = 0u; i < gameState.level.barriers.size(); ++i) {
        if (&gameState.level.barriers[i]->entity() == &entity) {
            return i;
        }
    }

    return -1u;
}
