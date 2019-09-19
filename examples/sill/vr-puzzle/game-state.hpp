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

enum class EditorState {
    Idle,
    MoveAlongAxis,
};

struct GameState {
    State state = State::Idle;

    lava::sill::GameEngine* engine = nullptr;
    lava::sill::CameraComponent* camera = nullptr;

    lava::sill::GameEntity* rayPickingEntity = nullptr;
    lava::sill::GameEntity* teleportBeamEntity = nullptr;
    lava::sill::GameEntity* teleportAreaEntity = nullptr;
    lava::Ray pickingRay;
    Brick* pointedBrick = nullptr;

    struct {
        std::string name;
        std::string path;

        std::vector<std::unique_ptr<Panel>> panels;
        std::vector<std::unique_ptr<Brick>> bricks;
        std::vector<lava::sill::GameEntity*> entities;
    } level;

    struct {
        lava::sill::GameEntity* selectedEntity = nullptr;
        lava::sill::GameEntity* gizmoEntity = nullptr;
        lava::sill::GameEntity* selectedGizmoAxis = nullptr;
        EditorState state = EditorState::Idle;

        // Used when moving along an axis
        glm::vec3 axis = {0.f, 0.f, 1.f};
        float axisOffset = 0.f;
    } editor;
};
