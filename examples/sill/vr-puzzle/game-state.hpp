#pragma once

#include "./brick.hpp"
#include "./panel.hpp"

#include <lava/core/ray.hpp>
#include <lava/sill.hpp>
#include <unordered_map>

struct GameState {
    lava::sill::GameEngine* engine = nullptr;
    lava::sill::CameraComponent* camera = nullptr;
    uint32_t levelId = 0u;

    lava::sill::GameEntity* rayPickingEntity = nullptr;
    lava::Ray pickingRay;

    // Bricks
    std::vector<std::unique_ptr<Panel>> panels;
    std::vector<std::unique_ptr<Brick>> bricks;
    Brick* pointedBrick = nullptr;

    lava::sill::GameEntity* wakingHall = nullptr;
};
