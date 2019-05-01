#include "./brick.hpp"

#include <iostream>

#include "./game-state.hpp"

using namespace lava;

Brick::Brick(GameState& gameState)
    : m_gameState(gameState)
{
    m_entity = &gameState.engine->make<sill::GameEntity>();
    m_entity->make<sill::TransformComponent>();
    m_entity->make<sill::AnimationComponent>();
    m_entity->make<sill::BoxColliderComponent>();
}

Brick::~Brick()
{
    m_gameState.engine->remove(*m_entity);

    for (auto& block : m_blocks) {
        m_gameState.engine->remove(*block.entity);
    }
}

void Brick::blocks(std::vector<glm::ivec2> blocks)
{
    // @fixme We should delete old meshes...
    m_blocks.resize(blocks.size());

    // Create blocks
    auto brickMaker = sill::makers::glbMeshMaker("./assets/models/vr-puzzle/puzzle-brick.glb");
    for (auto i = 0u; i < blocks.size(); ++i) {
        m_blocks[i].nonRotatedCoordinates = blocks[i];

        // Allocate block if needed
        if (m_blocks[i].entity == nullptr) {
            auto& entity = m_gameState.engine->make<sill::GameEntity>();
            auto& meshComponent = entity.make<sill::MeshComponent>();
            brickMaker(meshComponent);
            entity.get<sill::TransformComponent>().translate({glm::vec2(blocks[i]) * glm::vec2(blockExtent), 0});

            m_blocks[i].entity = &entity;
            m_entity->addChild(entity);
        }
    }

    // Update colliders
    glm::ivec2 minCoordinates = {100, 100};
    glm::ivec2 maxCoordinates = {-100, -100};
    for (auto i = 0u; i < blocks.size(); ++i) {
        if (blocks[i].x < minCoordinates.x) minCoordinates.x = blocks[i].x;
        if (blocks[i].y < minCoordinates.y) minCoordinates.y = blocks[i].y;
        if (blocks[i].x > maxCoordinates.x) maxCoordinates.x = blocks[i].x;
        if (blocks[i].y > maxCoordinates.y) maxCoordinates.y = blocks[i].y;
    }
    // @fixme Make better colliders with some sort of PhysicsComponent allowing multiple colliders at once.
    // Moreover, we can't offset this yet. Too bad.
    m_entity->get<sill::BoxColliderComponent>().dimensions(glm::vec3{(1.f + maxCoordinates.x - minCoordinates.x) * blockExtent.x,
                                                                     (1.f + maxCoordinates.y - minCoordinates.y) * blockExtent.y,
                                                                     blockExtent.z});

    // Update blocks
    m_baseRotationLevel = 0u;
    m_extraRotationLevel = 0u;

    updateBlocksColor();
    updateBlocksFromRotationLevel();
}

void Brick::color(const glm::vec3& color)
{
    m_color = color;

    updateBlocksColor();
}

void Brick::unsnap()
{
    if (m_snapPanel == nullptr) return;

    m_snapPanel = nullptr;
    m_entity->get<sill::BoxColliderComponent>().enabled(true);
}

void Brick::snap(const Panel& panel, const glm::uvec2& snapCoordinates)
{
    if (m_snapPanel == &panel && m_snapCoordinates == snapCoordinates) return;

    m_snapPanel = &panel;
    m_snapCoordinates = snapCoordinates;
    m_entity->get<sill::BoxColliderComponent>().enabled(false);
}

uint32_t Brick::incrementBaseRotationLevel()
{
    baseRotationLevel((m_baseRotationLevel + 1u) % 4u);
    return m_baseRotationLevel;
}

void Brick::baseRotationLevel(uint32_t baseRotationLevel)
{
    if (m_baseRotationLevel == baseRotationLevel) return;
    m_baseRotationLevel = baseRotationLevel;
    m_rotationLevel = m_baseRotationLevel + m_extraRotationLevel;
    updateBlocksFromRotationLevel();
}

void Brick::extraRotationLevel(uint32_t extraRotationLevel)
{
    if (m_extraRotationLevel == extraRotationLevel) return;
    m_extraRotationLevel = extraRotationLevel;
    m_rotationLevel = m_baseRotationLevel + m_extraRotationLevel;
    updateBlocksFromRotationLevel();
}

// Internal

void Brick::updateBlocksColor()
{
    for (auto& block : m_blocks) {
        block.entity->get<sill::MeshComponent>().node(1).mesh->primitive(0).material().set("albedoColor", m_color);
    }
}

void Brick::updateBlocksFromRotationLevel()
{
    for (auto& block : m_blocks) {
        block.coordinates = block.nonRotatedCoordinates;

        // X <= -Y and Y <=  X is a 90° clockwise rotation
        for (auto k = 0u; k < m_rotationLevel; ++k) {
            std::swap(block.coordinates.x, block.coordinates.y);
            block.coordinates.x = -block.coordinates.x;
        }
    }
}
