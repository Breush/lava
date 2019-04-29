#pragma once

#include "./panel.hpp"

#include <lava/sill.hpp>
#include <unordered_map>

struct Brick {
    lava::sill::GameEntity* entity = nullptr;

    // 0, 1, 2 or 3. Number of 90° counter-clockwise rotations to add.
    // rotationLevel = buttonRotationLevel + (hand rotation taken in account)
    uint32_t rotationLevel = 0u;
    uint32_t buttonRotationLevel = 0u;

    // Whether the entity is snapped to binding point and its coordinates if it is.
    bool snapped = false;
    glm::uvec2 snapCoordinates;

    // The pairs of all blocks making this brick,
    // 0,0 should always be present. The pairs are expressed
    // in relative X,Y coordinates at rotationLevel 0.
    std::vector<glm::ivec2> blocks; // These are updated each time the rotationLevel is changed.
    std::vector<glm::ivec2> nonRotatedBlocks;

    Brick(lava::sill::GameEntity* inEntity)
        : entity(inEntity)
    {
    }
};

struct BindingPoint {
    const lava::sill::MeshNode* node = nullptr;
    glm::uvec2 coordinates;
    bool filled = false;
};

struct GameState {
    lava::sill::GameEngine* engine = nullptr;
    uint32_t levelId = 0u;

    std::vector<Brick> bricks; // Bricks and such

    // Infos about the current table
    lava::sill::GameEntity* tableEntity = nullptr;
    lava::sill::Material* tableMaterial = nullptr;
    std::vector<std::vector<BindingPoint>> tableBindingPoints;

    Panel panel = Panel(3, 3);
    lava::sill::Material* panelMaterial = nullptr;
};
