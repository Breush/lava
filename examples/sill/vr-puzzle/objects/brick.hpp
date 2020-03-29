#pragma once

#include "./object.hpp"

#include <glm/glm.hpp>
#include <vector>

constexpr const glm::vec3 blockExtent = {0.22f, 0.22f, 0.0625f};

class Panel;
class Barrier;

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
class Brick : public Object {
public:
    Brick(GameState& gameState);
    void clear(bool removeFromLevel = true) final;
    float halfSpan() const final;

    // The pairs of all blocks making this brick,
    // 0,0 should always be present. The pairs are expressed
    // in relative X,Y coordinates at rotationLevel 0.
    const std::vector<Block>& blocks() const { return m_blocks; }
    void blocks(std::vector<glm::ivec2> blocks);

    // Add a block at position (x, ??).
    void addBlockV(int32_t x, bool positive);
    // Add a block at position (??, y).
    void addBlockH(int32_t y, bool positive);

    /// All barriers that prevents the brick from getting out.
    const std::set<Barrier*>& barriers() const { return m_barriers; }
    void addBarrier(Barrier& barrier) { m_barriers.emplace(&barrier); }
    void removeBarrier(Barrier& barrier);

    /// Checks whether the user is allowed to grab that brick or not.
    bool userInteractionAllowed() const;

    const glm::vec3& color() const { return m_color; }
    void color(const glm::vec3& color);

    /// Brick is hovered to be grabbed.
    bool selectionHighlighted() const { return m_selectionHighlighted; }
    void selectionHighlighted(bool selectionHighlighted);

    /// Brick is in error state.
    bool errorHighlighted() const { return m_errorHighlighted; }
    void errorHighlighted(bool errorHighlighted);

    /// Brick is locked and cannot be moved by player.
    bool fixed() const { return m_fixed; }
    void fixed(bool fixed);

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
    std::vector<Block> m_blocks;
    std::set<Barrier*> m_barriers;
    glm::vec3 m_color = {0.992, 0.992, 0.588};

    const Panel* m_snapPanel = nullptr;
    glm::uvec2 m_snapCoordinates = {0, 0};

    bool m_fixed = false;
    bool m_errorHighlighted = false;
    bool m_selectionHighlighted = false;
    uint32_t m_rotationLevel = 0u;
    uint32_t m_baseRotationLevel = 0u;
    uint32_t m_extraRotationLevel = 0u;
};

Brick* findBrick(GameState& gameState, const lava::sill::GameEntity& entity);
uint32_t findBrickIndex(GameState& gameState, const lava::sill::GameEntity& entity);
