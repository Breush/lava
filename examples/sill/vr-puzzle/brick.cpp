#include "./brick.hpp"

#include <iostream>

#include "./game-state.hpp"

using namespace lava;

Brick::Brick(GameState& gameState)
    : m_gameState(gameState)
{
    m_entity = &gameState.engine->make<sill::GameEntity>("brick");
    m_entity->make<sill::TransformComponent>();
    m_entity->make<sill::AnimationComponent>();
    m_entity->make<sill::ColliderComponent>();
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
    auto blockScaling = 0.75f;

    // Clean old entities...
    for (auto& block : m_blocks) {
        m_gameState.engine->remove(*block.entity);
    }
    m_blocks.resize(blocks.size());
    m_entity->get<sill::ColliderComponent>().clearShapes();

    // Create blocks
    static auto blockMaker = sill::makers::glbMeshMaker("./assets/models/vr-puzzle/puzzle-brick.glb");
    for (auto i = 0u; i < blocks.size(); ++i) {
        m_blocks[i].nonRotatedCoordinates = blocks[i];

        // Allocate block
        auto& entity = m_gameState.engine->make<sill::GameEntity>("block");
        auto& meshComponent = entity.make<sill::MeshComponent>();
        blockMaker(meshComponent);
        entity.get<sill::TransformComponent>().translate({glm::vec2(blocks[i]) * glm::vec2(blockExtent), 0});
        entity.get<sill::TransformComponent>().scale({blockScaling, blockScaling, 1.f});

        m_blocks[i].entity = &entity;
        m_entity->addChild(entity);
        m_entity->get<sill::ColliderComponent>().addBoxShape(
            {blocks[i].x * blockExtent.x, blocks[i].y * blockExtent.y, 0.5f * blockExtent.z},
            {blockExtent.x * blockScaling, blockExtent.y * blockScaling, blockExtent.z});
    }

    // Update blocks
    m_baseRotationLevel = 0u;
    m_extraRotationLevel = 0u;

    updateBlocksColor();
    updateBlocksFromRotationLevel();
}

void Brick::addBlockV(int32_t x, bool positive)
{
    std::vector<glm::ivec2> blocks;

    int32_t y = 0;
    for (auto block : m_blocks) {
        blocks.emplace_back(block.nonRotatedCoordinates);
        if (block.nonRotatedCoordinates.x != x) continue;
        if (positive && block.nonRotatedCoordinates.y >= y) {
            y = block.nonRotatedCoordinates.y + 1;
        }
        else if (!positive && block.nonRotatedCoordinates.y <= y) {
            y = block.nonRotatedCoordinates.y - 1;
        }
    }

    blocks.emplace_back(x, y);

    this->blocks(blocks);
}

void Brick::addBlockH(int32_t y, bool positive)
{
    std::vector<glm::ivec2> blocks;

    int32_t x = 0;
    for (auto block : m_blocks) {
        blocks.emplace_back(block.nonRotatedCoordinates);
        if (block.nonRotatedCoordinates.y != y) continue;
        if (positive && block.nonRotatedCoordinates.x >= x) {
            x = block.nonRotatedCoordinates.x + 1;
        }
        else if (!positive && block.nonRotatedCoordinates.x <= x) {
            x = block.nonRotatedCoordinates.x - 1;
        }
    }

    blocks.emplace_back(x, y);

    this->blocks(blocks);
}

bool Brick::userInteractionAllowed() const
{
    if (m_fixed) return false;

    auto playerPosition = glm::vec2(m_gameState.player.position);
    for (auto barrier : m_barriers) {
        if (!barrier->powered()) return false;

        auto barrierPosition = glm::vec2(barrier->transform().translation());
        if (glm::distance(playerPosition, barrierPosition) >= barrier->diameter() / 2.f) {
            return false;
        }
    }

    return true;
}

void Brick::color(const glm::vec3& color)
{
    m_color = color;

    updateBlocksColor();
}

void Brick::selectionHighlighted(bool selectionHighlighted) {
    if (m_selectionHighlighted == selectionHighlighted) return;
    m_selectionHighlighted = selectionHighlighted;

    updateBlocksColor();
}

void Brick::errorHighlighted(bool errorHighlighted) {
    if (m_errorHighlighted == errorHighlighted) return;
    m_errorHighlighted = errorHighlighted;

    updateBlocksColor();
}

void Brick::fixed(bool fixed) {
    if (m_fixed == fixed) return;
    m_fixed = fixed;

    updateBlocksColor();
}

void Brick::unsnap()
{
    if (m_snapPanel == nullptr) return;

    m_snapPanel = nullptr;
    m_entity->get<sill::PhysicsComponent>().enabled(true);
}

void Brick::snap(const Panel& panel, const glm::uvec2& snapCoordinates)
{
    if (m_snapPanel == &panel && m_snapCoordinates == snapCoordinates) return;

    m_snapPanel = &panel;
    m_snapCoordinates = snapCoordinates;
    m_entity->get<sill::PhysicsComponent>().enabled(false);
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
    auto color = m_color;

    if (m_fixed) {
        color = glm::vec3(0.8f);
    }
    else if (m_errorHighlighted) {
        color = glm::mix(color, glm::vec3(0.95f, 0.2f, 0.2f), 0.75f);
    }
    else if (m_selectionHighlighted) {
        color = glm::mix(color, glm::vec3(0.4f, 0.4f, 0.95f), 0.75f);
    }

    for (auto& block : m_blocks) {
        block.entity->get<sill::MeshComponent>().material(1, 0)->set("albedoColor", color);
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

// -----

Brick* findBrick(GameState& gameState, const sill::GameEntity& entity)
{
    for (const auto& brick : gameState.level.bricks) {
        if (&brick->entity() == &entity) {
            return brick.get();
        }
    }

    return nullptr;
}

uint32_t findBrickIndex(GameState& gameState, const sill::GameEntity& entity)
{
    for (auto i = 0u; i < gameState.level.bricks.size(); ++i) {
        if (&gameState.level.bricks[i]->entity() == &entity) {
            return i;
        }
    }

    return -1u;
}
