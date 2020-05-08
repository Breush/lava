#include "./brick.hpp"

#include <algorithm>

#include "../game-state.hpp"
#include "./pedestal.hpp"

using namespace lava;

Brick::Brick(GameState& gameState)
    : Object(gameState)
{
    m_entity = &gameState.engine->make<sill::GameEntity>("brick");
    m_entity->ensure<sill::TransformComponent>();
    m_entity->ensure<sill::AnimationComponent>();
    m_entity->ensure<sill::MeshComponent>();
}

void Brick::clear(bool removeFromLevel)
{
    Object::clear(removeFromLevel);

    if (removeFromLevel) {
        auto brickIt = std::find_if(m_gameState.level.bricks.begin(), m_gameState.level.bricks.end(), [this](const std::unique_ptr<Brick>& brick) {
            return (brick.get() == this);
        });
        m_gameState.level.bricks.erase(brickIt);
    }
}

float Brick::halfSpan() const
{
    glm::ivec2 minBlock{0};
    glm::ivec2 maxBlock{0};
    for (const auto& block : m_blocks) {
        minBlock = glm::max(minBlock, block.nonRotatedCoordinates);
        maxBlock = glm::max(maxBlock, block.nonRotatedCoordinates);
    }

    auto brickHalfSpan = std::max(blockExtent.x * (std::max(maxBlock.x, std::abs(minBlock.x)) + 1),
                                  blockExtent.y * (std::max(maxBlock.y, std::abs(minBlock.y)) + 1));

    return brickHalfSpan;
}

void Brick::blocks(std::vector<glm::ivec2> blocks)
{
    constexpr const auto blockScaling = 0.75f;

    // Clean
    m_blocks.resize(blocks.size());

    // Create blocks
    static auto blockMaker = sill::makers::glbMeshMaker("./assets/models/vr-puzzle/puzzle-brick.glb");
    for (auto i = 0u; i < blocks.size(); ++i) {
        m_blocks[i].nonRotatedCoordinates = blocks[i];

        // Make block
        blockMaker(mesh());
        m_blocks[i].meshNodeIndex = mesh().nodes().size() - 1u;

        auto& rootNode = mesh().node(m_blocks[i].meshNodeIndex - 1u);
        auto transform = glm::mat4(1.f);
        transform = glm::translate(transform, {glm::vec2(blocks[i]) * glm::vec2(blockExtent), 0.f});
        transform = glm::scale(transform, {blockScaling, blockScaling, 1.f});
        rootNode.transform(transform * rootNode.transform());
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

void Brick::removeBarrier(Barrier& barrier)
{
    auto barrierIt = m_barriers.find(&barrier);
    if (barrierIt == m_barriers.end()) return;
    m_barriers.erase(barrierIt);
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

void Brick::stored(bool stored)
{
    if (m_stored == stored) return;
    m_stored = stored;

    if (m_pedestal) {
        m_pedestal->brickStoredChanged(*this);
    }
}

void Brick::unsnap()
{
    if (m_snapPanel == nullptr) return;

    auto snapPanel = m_snapPanel;
    m_snapPanel = nullptr;

    snapPanel->snappedBricksChanged();
}

void Brick::snap(Panel& panel, const glm::uvec2& snapCoordinates)
{
    if (m_snapPanel == &panel && m_snapCoordinates == snapCoordinates) return;

    m_snapPanel = &panel;
    m_snapCoordinates = snapCoordinates;
    m_snapPanel->snappedBricksChanged();
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
        mesh().material(block.meshNodeIndex, 0u)->set("albedoColor", color);
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
