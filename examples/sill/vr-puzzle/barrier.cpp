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
    m_entity->make<sill::ColliderComponent>();

    auto& meshComponent = m_entity->get<sill::MeshComponent>();
    sill::makers::CylinderMeshOptions options = {.doubleSided = true, .offset = 1.5f};
    sill::makers::cylinderMeshMaker(8u, 1.f, 3.f, options)(meshComponent);
    meshComponent.primitive(0u, 0u).material(*gameState.barrierMaterial);
    meshComponent.primitive(0u, 0u).category(RenderCategory::Translucent);
    meshComponent.primitive(0u, 0u).shadowsCastable(false);
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

// -----

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
