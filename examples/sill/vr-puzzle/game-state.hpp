#pragma once

#include <lava/sill.hpp>
#include <unordered_map>

struct Brick {
    lava::sill::GameEntity* entity = nullptr;
    uint32_t rotationLevel = 0u; // 0, 1, 2 or 3. Number of 90° rotations to add.
    bool snapped = false;        // Whether the entity is snapped to binding point.

    Brick(lava::sill::GameEntity* inEntity)
        : entity(inEntity)
    {
    }
};

struct GameState {
    lava::sill::GameEngine* engine = nullptr;
    std::vector<Brick> bricks; // Bricks and such

    // Infos about the current table
    lava::sill::Material* tablePanelMaterial = nullptr;
    std::vector<std::vector<const lava::sill::MeshNode*>> tableBindingNodes;
};
