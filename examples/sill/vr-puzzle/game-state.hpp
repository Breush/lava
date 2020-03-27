#pragma once

#include "./barrier.hpp"
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
    TranslateAlongAxis,
    RotateAlongAxis,
};

enum class CameraMode {
    FirstPerson,
    Orbit,
};

enum class GizmoTool {
    Translation,
    Rotation,
    // @todo Scaling, not implemented yet
};

struct GameState {
    State state = State::Idle;

    lava::sill::GameEngine* engine = nullptr;

    lava::sill::GameEntity* rayPickingEntity = nullptr;
    lava::sill::GameEntity* teleportBeamEntity = nullptr;
    lava::sill::GameEntity* teleportAreaEntity = nullptr;
    lava::Ray pickingRay;
    Brick* pointedBrick = nullptr;

    struct {
        CameraMode mode = CameraMode::FirstPerson;
        lava::sill::CameraComponent* component = nullptr;
        lava::sill::GameEntity* reticleEntity = nullptr;
        bool reticleUpdateNeeded = true;
    } camera;

    struct {
        std::string name;
        std::string path;

        std::vector<std::unique_ptr<Panel>> panels;
        std::vector<std::unique_ptr<Brick>> bricks;
        std::vector<std::unique_ptr<Barrier>> barriers;
        std::vector<lava::sill::GameEntity*> entities;
    } level;

    struct {
        EditorState state = EditorState::Idle;
        std::vector<lava::sill::GameEntity*> selectedEntities;

        struct {
            GizmoTool tool = GizmoTool::Translation;
            lava::sill::GameEntity* entity = nullptr; // Holding all tool gizmos (translation/rotation/scaling)
            lava::sill::GameEntity* toolEntity = nullptr; // One tool from list below
            lava::sill::GameEntity* translationToolEntity = nullptr;
            lava::sill::GameEntity* rotationToolEntity = nullptr;
            lava::sill::GameEntity* selectedToolAxis = nullptr;

            // Used when moving along an axis
            glm::vec3 axis = {0.f, 0.f, 1.f};
            glm::vec3 nextAxis = {1.f, 0.f, 0.f};
            float axisOffset = 0.f;
        } gizmo;
    } editor;
};
