#pragma once

#include "./brick.hpp"
#include "./panel.hpp"

#include <lava/sill.hpp>
#include <unordered_map>

struct GameState {
    lava::sill::GameEngine* engine = nullptr;
    uint32_t levelId = 0u;

    lava::sill::GameEntity* rayPickingEntity = nullptr;

    // Bricks
    std::vector<Brick> bricks;
    Brick* pointedBrick = nullptr;

    // Current table
    Panel panel;
    lava::sill::GameEntity* tableEntity = nullptr;
    lava::sill::Material* tableMaterial = nullptr;
};
