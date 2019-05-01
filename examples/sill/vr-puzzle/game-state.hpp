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
    std::vector<std::unique_ptr<Panel>> panels;
    std::vector<std::unique_ptr<Brick>> bricks;
    Brick* pointedBrick = nullptr;
};
