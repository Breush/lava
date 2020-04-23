#pragma once

#include "./objects/barrier.hpp"
#include "./objects/brick.hpp"
#include "./objects/generic.hpp"
#include "./objects/panel.hpp"

#include <lava/core/ray.hpp>
#include <lava/sill.hpp>
#include <lava/magma.hpp>
#include <unordered_map>

enum class State {
    Idle,
    Editor,
    GrabbedBrick,
    TeleportBeam,
};

enum class EditorState {
    Idle,
    MultiSelection,
    TranslateAlongAxis,
    RotateAlongAxis,
    ScaleAlongAxis,
};

enum class CameraMode {
    FirstPerson,
    Orbit,
};

enum class GizmoTool {
    Translation,
    Rotation,
    Scaling,
};

struct GameState {
    State state = State::Idle;

    lava::sill::GameEngine* engine = nullptr;

    lava::sill::GameEntity* rayPickingEntity = nullptr;
    lava::Ray pickingRay;
    Brick* pointedBrick = nullptr;

    struct {
        std::vector<lava::sill::GameEntity*> entities;
    } ui;

    struct {
        Panel* panel;
        glm::uvec2 coordinates;
    } snapping;

    struct {
        glm::vec3 position; // Camera projection on terrain in FPS, area position in VR.
        glm::vec3 direction = {1.f, 0.f, 0.f}; // Always normalized, not used in VR.
        glm::vec3 headPosition; // Camera in FPS, headset in VR.
    } player;

    struct {
        lava::sill::GameEntity* entity = nullptr;
    } terrain;

    struct {
        lava::sill::GameEntity* beamEntity = nullptr;
        lava::sill::GameEntity* areaEntity = nullptr;
        glm::vec3 target = glm::vec3(0.f);
        bool valid = true;
    } teleport;

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
        std::vector<std::unique_ptr<Generic>> generics;

        // All objects (including panels/bricks/barriers/generics)
        std::vector<Object*> objects;
    } level;

    struct {
        EditorState state = EditorState::Idle;

        struct {
            std::vector<Object*> objects;
            glm::vec2 multiStart;
            glm::vec2 multiEnd;
            lava::sill::GameEntity* multiEntity = nullptr; // The rectangle
        } selection;

        struct {
            // @todo Move that to general overlay scene?
            lava::magma::Scene* scene = nullptr;
            lava::magma::Camera* camera = nullptr;

            GizmoTool tool = GizmoTool::Translation;
            lava::sill::GameEntity* entity = nullptr; // Holding all tool gizmos (translation/rotation/scaling)
            lava::sill::GameEntity* toolEntity = nullptr; // One tool from list below
            lava::sill::GameEntity* translationToolEntity = nullptr;
            lava::sill::GameEntity* rotationToolEntity = nullptr;
            lava::sill::GameEntity* scalingToolEntity = nullptr;
            lava::sill::GameEntity* selectedToolAxis = nullptr;

            // Used when moving along an axis
            glm::vec3 axis = {0.f, 0.f, 1.f};
            glm::vec3 nextAxis = {1.f, 0.f, 0.f};
            float axisOffset = 0.f;
            float previousScaling = 1.f;
        } gizmo;
    } editor;
};
