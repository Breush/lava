#pragma once

#include <glm/glm.hpp>
#include <lava/sill.hpp>
#include <vector>

constexpr const glm::vec3 blockExtent = {0.22f, 0.22f, 0.0625f};

class GameState;
class Panel;

struct Block {
    // These are updated each time the rotationLevel is changed.
    glm::ivec2 coordinates = {0, 0};
    glm::ivec2 nonRotatedCoordinates = {0, 0};
    lava::sill::GameEntity* entity = nullptr;
};

/**
 * A brick is composed of multiple blocks.
 * Making it be something like a tetris shape.
 */
class Brick {
public:
    Brick(GameState& gameState);
    ~Brick();

    lava::sill::AnimationComponent& animation() { return m_entity->get<lava::sill::AnimationComponent>(); }
    lava::sill::TransformComponent& transform() { return m_entity->get<lava::sill::TransformComponent>(); }

    // The pairs of all blocks making this brick,
    // 0,0 should always be present. The pairs are expressed
    // in relative X,Y coordinates at rotationLevel 0.
    const std::vector<Block>& blocks() const { return m_blocks; }
    void blocks(std::vector<glm::ivec2> blocks);

    const glm::vec3& color() const { return m_color; }
    void color(const glm::vec3& color);

    // Whether the entity is snapped to snapping point and its coordinates if it is.
    bool snapped() const { return m_snapPanel != nullptr; }
    const Panel& snapPanel() const { return *m_snapPanel; }
    const glm::uvec2& snapCoordinates() const { return m_snapCoordinates; }
    void unsnap();
    void snap(const Panel& panel, const glm::uvec2& snapCoordinates);

    // 0, 1, 2 or 3. Number of 90° counter-clockwise rotations to add.
    // rotationLevel = baseRotationLevel + extraRotationLevel
    // Blocks coordinates will always be in sync with rotationLevel.
    uint32_t incrementBaseRotationLevel();
    uint32_t baseRotationLevel() const { return m_baseRotationLevel; }
    uint32_t rotationLevel() const { return m_rotationLevel; }
    void baseRotationLevel(uint32_t baseRotationLevel);
    void extraRotationLevel(uint32_t extraRotationLevel);

protected:
    void updateBlocksColor();
    void updateBlocksFromRotationLevel();

private:
    GameState& m_gameState;
    lava::sill::GameEntity* m_entity = nullptr;

    std::vector<Block> m_blocks;
    glm::vec3 m_color = {1, 1, 1};

    const Panel* m_snapPanel = nullptr;
    glm::uvec2 m_snapCoordinates = {0, 0};

    uint32_t m_rotationLevel = 0u;
    uint32_t m_baseRotationLevel = 0u;
    uint32_t m_extraRotationLevel = 0u;
};
