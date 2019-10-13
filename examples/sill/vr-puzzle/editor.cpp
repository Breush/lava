#include "./editor.hpp"

#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <lava/chamber/math.hpp>
#include <lava/dike.hpp>
#include <lava/magma.hpp>

#include "./brick.hpp"
#include "./environment.hpp"
#include "./serializer.hpp"

using namespace lava;
using namespace lava::chamber;

namespace {
    std::array<glm::vec3, 3u> g_axes = {glm::vec3{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

    void setCategoryForHierarchy(sill::GameEntity* entity, RenderCategory category)
    {
        if (!entity) return;

        if (entity->has<sill::MeshComponent>()) {
            entity->get<sill::MeshComponent>().category(category);
        }

        for (auto& child : entity->children()) {
            setCategoryForHierarchy(child, category);
        }
    }
}

void setupEditor(GameState& gameState)
{
    auto& engine = *gameState.engine;

    // Inputs

    engine.input().bindAction("up", Key::Up);
    engine.input().bindAction("down", Key::Down);
    engine.input().bindAction("left", Key::Left);
    engine.input().bindAction("right", Key::Right);
    engine.input().bindAction("save", {Key::LeftControl, Key::S});
    engine.input().bindAction("reload-level", {Key::LeftControl, Key::R});

    // @fixme This is for debug for now
    engine.input().bindAction("add-panel", {Key::LeftShift, Key::A, Key::P});
    engine.input().bindAction("add-brick", {Key::LeftShift, Key::A, Key::B});
    engine.input().bindAction("add-mesh", {Key::LeftShift, Key::A, Key::M});
    engine.input().bindAction("rotate-z", Key::R);
    engine.input().bindAction("rotate-x", {Key::LeftShift, Key::R});
    engine.input().bindAction("scale-up", Key::S);
    engine.input().bindAction("scale-down", {Key::LeftShift, Key::S});

    auto& editorEntity = engine.make<sill::GameEntity>("editor");
    auto& editorBehavior = editorEntity.make<sill::BehaviorComponent>();

    // Gizmo
    {
        auto& gizmoEntity = engine.make<sill::GameEntity>("gizmo");
        gameState.editor.gizmoEntity = &gizmoEntity;

        for (const auto& axis : g_axes) {
            auto& axisEntity = engine.make<sill::GameEntity>("gizmo-axis");
            auto& axisMaterial = engine.scene().make<magma::Material>("gizmo");
            axisMaterial.set("color", axis);

            auto& axisMeshComponent = axisEntity.make<sill::MeshComponent>();
            sill::makers::CylinderMeshOptions options;
            options.transform = glm::rotate(glm::mat4(1.f), math::PI_OVER_TWO, {0, 0, 1});
            options.transform = glm::rotate(options.transform, math::PI_OVER_TWO, axis);
            options.offset = 0.25f;
            sill::makers::cylinderMeshMaker(8u, 0.02f, 0.5f, options)(axisMeshComponent);
            axisMeshComponent.primitive(0, 0).material(axisMaterial);
            axisMeshComponent.primitive(0, 0).shadowsCastable(false);

            gizmoEntity.addChild(axisEntity);
        }

        // @fixme There's an engine bug where doing that before adding children,
        // won't update children's transforms.
        gizmoEntity.make<sill::TransformComponent>().scaling(glm::vec3(0.f));
    }

    engine.input().bindAction("editorToggle", Key::E);

    editorBehavior.onUpdate([&](float /* dt */) {
        if ((gameState.state == State::Idle || gameState.state == State::Editor) && engine.input().justDown("editorToggle")) {
            gameState.state = (gameState.state == State::Editor) ? State::Idle : State::Editor;
            gameState.editor.state = EditorState::Idle;

            engine.physicsEngine().enabled(gameState.state != State::Editor);
            gameState.editor.gizmoEntity->get<sill::TransformComponent>().scaling(glm::vec3(0.f));

            std::cout << "Editor: " << (gameState.state == State::Editor) << std::endl;
        }

        if (gameState.state != State::Editor) return;

        if (gameState.editor.state == EditorState::Idle) {
            // ----- Gizmos

            float minGizmoAxisDistance = INFINITY;
            sill::GameEntity* selectedGizmoAxis = nullptr;
            for (auto gizmoChild : gameState.editor.gizmoEntity->children()) {
                float gizmoChildDistance = gizmoChild->distanceFrom(gameState.pickingRay);
                if (gizmoChildDistance <= 0.f) continue;

                if (gizmoChildDistance < minGizmoAxisDistance && gizmoChild->name() == "gizmo-axis") {
                    minGizmoAxisDistance = gizmoChildDistance;
                    selectedGizmoAxis = gizmoChild;
                    break;
                }
            }

            // Display highlight for gizmos
            if (gameState.editor.selectedGizmoAxis) {
                gameState.editor.selectedGizmoAxis->get<sill::MeshComponent>().material(0, 0)->set("highlight", false);
            }
            gameState.editor.selectedGizmoAxis = selectedGizmoAxis;
            if (selectedGizmoAxis) {
                selectedGizmoAxis->get<sill::MeshComponent>().material(0, 0)->set("highlight", true);
            }

            // ----- Add

            if (engine.input().justDown("add-panel")) {
                auto panel = std::make_unique<Panel>(gameState);
                panel->extent({3, 3});
                gameState.level.panels.emplace_back(std::move(panel));
                return;
            }
            else if (engine.input().justDown("add-brick")) {
                auto brick = std::make_unique<Brick>(gameState);
                brick->blocks({{0, 0}});
                brick->color({1, 0, 0});
                gameState.level.bricks.emplace_back(std::move(brick));
                return;
            }
            else if (engine.input().justDown("add-mesh")) {
                std::cout << "Available meshes:" << std::endl;
                for (const auto& entry : std::filesystem::directory_iterator("./assets/models/vr-puzzle/")) {
                    if (entry.path().extension() == ".glb") {
                        std::cout << entry.path() << std::endl;
                    }
                }

                std::string fileName;
                std::cout << "Please enter a .glb file name (without path nor extension):" << std::endl;
                std::cin >> fileName;
                auto filePath = "./assets/models/vr-puzzle/" + fileName + ".glb";

                auto& entity = gameState.engine->make<sill::GameEntity>(fileName);
                auto& meshComponent = entity.make<sill::MeshComponent>();
                sill::makers::glbMeshMaker(filePath)(meshComponent);
                gameState.level.entities.emplace_back(&entity);
                return;
            }

            // ----- Reload

            if (engine.input().justDown("reload-level")) {
                loadLevel(gameState, gameState.level.path);
                gameState.editor.selectedEntity = nullptr;
                gameState.editor.gizmoEntity->get<sill::TransformComponent>().scaling(glm::vec3(0.f));
                return;
            }

            // ----- Left click

            if (engine.input().justDown("left-fire")) {
                // Go the gizmo mode if needed
                if (gameState.editor.selectedGizmoAxis) {
                    uint32_t axisIndex = gameState.editor.gizmoEntity->childIndex(*gameState.editor.selectedGizmoAxis);
                    gameState.editor.state = EditorState::MoveAlongAxis;
                    gameState.editor.axis = g_axes[axisIndex];

                    Ray axisRay;
                    axisRay.origin = gameState.editor.selectedEntity->get<sill::TransformComponent>().translation();
                    axisRay.direction = gameState.editor.axis;
                    gameState.editor.axisOffset = projectOn(axisRay, gameState.pickingRay);
                    return;
                }

                // Select entity on left click if any.
                auto* selectedEntity = engine.pickEntity(gameState.pickingRay);

                if (selectedEntity) {
                    // Find the "root" entity of the selected mesh.
                    while (selectedEntity->parent()) {
                        selectedEntity = selectedEntity->parent();
                    }
                }
                else {
                    gameState.editor.gizmoEntity->get<sill::TransformComponent>().scaling(glm::vec3(0.f));
                }

                gameState.editor.selectedEntity = selectedEntity;
            }
            else if (engine.input().justDown("save")) {
                serializeLevel(gameState, gameState.level.path);
                return;
            }
        }

        // ----- Selected entity control

        if (!gameState.editor.selectedEntity) return;

        const auto& translation = gameState.editor.selectedEntity->get<sill::TransformComponent>().translation();
        gameState.editor.gizmoEntity->get<sill::TransformComponent>().translation(translation);

        const auto gizmoDistanceToCamera = std::sqrt(glm::length(gameState.camera->origin() - translation));
        gameState.editor.gizmoEntity->get<sill::TransformComponent>().scaling(glm::vec3(gizmoDistanceToCamera));

        // Move entity if needed.
        if (gameState.editor.state == EditorState::MoveAlongAxis) {
            if (engine.input().justUp("left-fire")) {
                gameState.editor.state = EditorState::Idle;
                return;
            }

            if (engine.input().axisChanged("main-x") || engine.input().axisChanged("main-y")) {
                const Ray axisRay = {.origin = translation, .direction = gameState.editor.axis};
                const auto delta = projectOn(axisRay, gameState.pickingRay) - gameState.editor.axisOffset;
                gameState.editor.selectedEntity->get<sill::TransformComponent>().translate(-delta * gameState.editor.axis);
            }
        }
        else if (gameState.editor.state == EditorState::Idle) {
            if (engine.input().justDown("rotate-z") && !engine.input().down("rotate-x")) { // R but not when Shift + R
                gameState.editor.selectedEntity->get<sill::TransformComponent>().rotate({0, 0, 1}, math::PI_OVER_FOUR);
            }
            if (engine.input().justDown("rotate-x")) {
                gameState.editor.selectedEntity->get<sill::TransformComponent>().rotate({1, 0, 0}, math::PI_OVER_FOUR);
            }
            if (engine.input().justDown("scale-down")) {
                gameState.editor.selectedEntity->get<sill::TransformComponent>().scale(0.9f);
            }
            if (engine.input().justDown("scale-up")) {
                gameState.editor.selectedEntity->get<sill::TransformComponent>().scale(1.1f);
            }

            if (gameState.editor.selectedEntity->name() == "brick") {
                if (engine.input().justDown("up")) {
                    findBrick(gameState, *gameState.editor.selectedEntity).addBlockV(0, true);
                }
                if (engine.input().justDown("down")) {
                    findBrick(gameState, *gameState.editor.selectedEntity).addBlockV(0, false);
                }
                if (engine.input().justDown("right")) {
                    findBrick(gameState, *gameState.editor.selectedEntity).addBlockH(0, true);
                }
                if (engine.input().justDown("left")) {
                    findBrick(gameState, *gameState.editor.selectedEntity).addBlockH(0, false);
                }
            }
        }
    });
}
