#pragma once

#include "./brick.hpp"
#include "./panel.hpp"

#include <lava/core/ray.hpp>
#include <lava/sill.hpp>
#include <unordered_map>

enum class State {
    Idle,
    Editor,
    GrabbedBrick,
    TeleportBeam,
};

struct GameState {
    State state = State::Idle;

    lava::sill::GameEngine* engine = nullptr;
    lava::sill::CameraComponent* camera = nullptr;
    uint32_t levelId = 0u;

    lava::sill::GameEntity* rayPickingEntity = nullptr;
    lava::sill::GameEntity* teleportBeamEntity = nullptr;
    lava::sill::GameEntity* teleportAreaEntity = nullptr;
    lava::Ray pickingRay;

    // Bricks
    std::vector<std::unique_ptr<Panel>> panels;
    std::vector<std::unique_ptr<Brick>> bricks;
    Brick* pointedBrick = nullptr;

    lava::sill::GameEntity* wakingHall = nullptr;

    struct {
        lava::sill::GameEntity* selectedEntity = nullptr;
    } editor;
};
