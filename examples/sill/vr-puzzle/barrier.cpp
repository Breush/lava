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
    sill::makers::CylinderMeshOptions options = {.doubleSided = true, .offset = 1.f};
    sill::makers::cylinderMeshMaker(8u, 1.5f, 2.f, options)(meshComponent);
    meshComponent.primitive(0u, 0u).material(*gameState.barrierMaterial);
    meshComponent.primitive(0u, 0u).category(RenderCategory::Translucent);
    meshComponent.primitive(0u, 0u).shadowsCastable(false);
}

Barrier::~Barrier()
{
    m_gameState.engine->remove(*m_entity);
}
