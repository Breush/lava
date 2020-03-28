#include "./editor.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>
#include <lava/chamber/math.hpp>
#include <lava/dike.hpp>
#include <lava/magma.hpp>

#include "./brick.hpp"
#include "./camera.hpp"
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

    float rotationAxisOffset(GameState& gameState, const glm::vec3& origin) {
        const Ray axisRay = {.origin = origin, .direction = gameState.editor.gizmo.axis};
        float distance = intersectPlane(gameState.pickingRay, axisRay);
        auto intersection = gameState.pickingRay.origin + distance * gameState.pickingRay.direction;
        auto unitDirection = glm::normalize(intersection - axisRay.origin);
        return glm::orientedAngle(unitDirection, gameState.editor.gizmo.nextAxis, axisRay.direction);
    }
}

void setGizmoTool(GameState& gameState, GizmoTool gizmoTool) {
    gameState.editor.gizmo.tool = gizmoTool;

    if (gameState.editor.gizmo.tool == GizmoTool::Translation) {
        gameState.editor.gizmo.toolEntity = gameState.editor.gizmo.translationToolEntity;
        gameState.editor.gizmo.translationToolEntity->get<sill::TransformComponent>().scaling(1.f);
        gameState.editor.gizmo.rotationToolEntity->get<sill::TransformComponent>().scaling(0.f);
    }
    else if (gameState.editor.gizmo.tool == GizmoTool::Rotation) {
        gameState.editor.gizmo.toolEntity = gameState.editor.gizmo.rotationToolEntity;
        gameState.editor.gizmo.translationToolEntity->get<sill::TransformComponent>().scaling(0.f);
        gameState.editor.gizmo.rotationToolEntity->get<sill::TransformComponent>().scaling(1.f);
    }
}

void unselectAllEntities(GameState& gameState) {
    gameState.editor.selectedEntities.clear();
    gameState.editor.gizmo.entity->get<sill::TransformComponent>().scaling(0.f);
}

void selectEntity(GameState& gameState, sill::GameEntity* entity, bool addToSelection) {
    if (!entity) {
        if (!addToSelection) {
            unselectAllEntities(gameState);
        }
        return;
    }

    // Find the "root" entity of the selected mesh.
    while (entity->parent()) {
        entity = entity->parent();
    }

    // If we already selected that entity, ignore
    auto selectedEntityIt = std::find(gameState.editor.selectedEntities.begin(), gameState.editor.selectedEntities.end(), entity);
    if (selectedEntityIt != gameState.editor.selectedEntities.end()) {
        return;
    }

    if (!addToSelection) {
        unselectAllEntities(gameState);
    }

    gameState.editor.selectedEntities.emplace_back(entity);

    // @todo Leaving this message as long as we don't have a highlight on selection
    if (gameState.editor.selectedEntities.size() > 1u) {
        std::cout << "Selected " << gameState.editor.selectedEntities.size() << " entities." << std::endl;
    }
}

// Specific behavior based on what it selected.
void updateSelectedEntity(GameState& gameState, sill::GameEntity& entity)
{
    auto& engine = *gameState.engine;

    if (gameState.editor.selectedEntities.size() > 1u) return;

    // Following actions are only interesting when there one object selected.
    if (entity.name() == "brick") {
        auto brick = findBrick(gameState, entity);
        if (engine.input().justDown("up")) {
            brick->addBlockV(0, true);
        }
        if (engine.input().justDown("down")) {
            brick->addBlockV(0, false);
        }
        if (engine.input().justDown("right")) {
            brick->addBlockH(0, true);
        }
        if (engine.input().justDown("left")) {
            brick->addBlockH(0, false);
        }
        if (engine.input().justDown("brick.toggle-fixed")) {
            brick->fixed(!brick->fixed());
        }
    }
    else if (entity.name() == "barrier") {
        auto barrier = findBarrier(gameState, entity);
        if (engine.input().justDown("up")) {
            barrier->diameter(barrier->diameter() + 0.5f);
        }
        if (engine.input().justDown("down")) {
            barrier->diameter(barrier->diameter() - 0.5f);
        }
        if (engine.input().justDown("left") || engine.input().justDown("right")) {
            barrier->powered(!barrier->powered());
        }
    }
    else if (entity.name() == "panel") {
        auto panel = findPanel(gameState, entity);
        if (engine.input().justDown("up")) {
            panel->extent(panel->extent() + glm::uvec2(0, 1));
        }
        if (engine.input().justDown("down")) {
            panel->extent(panel->extent() - glm::uvec2(0, 1));
        }
        if (engine.input().justDown("right")) {
            panel->extent(panel->extent() + glm::uvec2(1, 0));
        }
        if (engine.input().justDown("left")) {
            panel->extent(panel->extent() - glm::uvec2(1, 0));
        }
    }
}

void setupEditor(GameState& gameState)
{
    auto& engine = *gameState.engine;

    // Inputs

    engine.input().bindAction("toggle-editor", Key::E);
    engine.input().bindAction("editor.switch-gizmo", MouseButton::Middle);
    engine.input().bindAction("editor.multiple-selection-modifier", {Key::LeftShift});
    engine.input().bindAction("editor.multiple-selection-modifier", {Key::RightShift});

    engine.input().bindAction("up", Key::Up);
    engine.input().bindAction("down", Key::Down);
    engine.input().bindAction("left", Key::Left);
    engine.input().bindAction("right", Key::Right);
    engine.input().bindAction("save", {Key::LeftControl, Key::S});
    engine.input().bindAction("reload-level", {Key::LeftControl, Key::R});

    // @fixme This is for debug for now
    engine.input().bindAction("add-panel", {Key::LeftShift, Key::A, Key::P});
    engine.input().bindAction("add-brick", {Key::LeftShift, Key::A, Key::B});
    engine.input().bindAction("add-barrier", {Key::LeftShift, Key::A, Key::R});
    engine.input().bindAction("add-mesh", {Key::LeftShift, Key::A, Key::M});
    engine.input().bindAction("brick.toggle-fixed", {Key::LeftShift, Key::F});
    engine.input().bindAction("bind-to-barrier", {Key::LeftAlt, Key::R});
    // @fixme Disabling, need gizmo
    // engine.input().bindAction("scale-up", Key::S);
    // engine.input().bindAction("scale-down", {Key::LeftShift, Key::S});
    // @todo Make action to switch to rotation gizmo on R

    auto& editorEntity = engine.make<sill::GameEntity>("editor");
    auto& editorBehavior = editorEntity.make<sill::BehaviorComponent>();

    // Gizmo
    {
        auto& gizmoEntity = engine.make<sill::GameEntity>("gizmo");
        gizmoEntity.ensure<sill::TransformComponent>();
        gameState.editor.gizmo.entity = &gizmoEntity;

        // --- Translation
        auto& translationToolEntity = engine.make<sill::GameEntity>("gizmo-translation");
        translationToolEntity.ensure<sill::TransformComponent>();
        gizmoEntity.addChild(translationToolEntity);
        gameState.editor.gizmo.translationToolEntity = &translationToolEntity;

        for (const auto& axis : g_axes) {
            auto& axisEntity = engine.make<sill::GameEntity>("gizmo-translation-axis");
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

            translationToolEntity.addChild(axisEntity);
        }

        // --- Rotation
        auto& rotationToolEntity = engine.make<sill::GameEntity>("gizmo-rotation");
        rotationToolEntity.ensure<sill::TransformComponent>();
        gizmoEntity.addChild(rotationToolEntity);
        gameState.editor.gizmo.rotationToolEntity = &rotationToolEntity;

        for (const auto& axis : g_axes) {
            auto& axisEntity = engine.make<sill::GameEntity>("gizmo-rotation-axis");
            auto& axisMaterial = engine.scene().make<magma::Material>("gizmo");
            axisMaterial.set("color", axis);

            auto& axisMeshComponent = axisEntity.make<sill::MeshComponent>();
            auto transform = glm::rotate(glm::mat4(1.f), math::PI_OVER_TWO, {0, 0, 1});
            transform = glm::rotate(transform, math::PI_OVER_TWO, axis);
            sill::makers::toreMeshMaker(32u, 1.f, 4u, 0.02f)(axisMeshComponent);
            axisMeshComponent.node(0).localTransform = transform; // @todo Have a generic way to bake transform to vertices
            axisMeshComponent.primitive(0, 0).material(axisMaterial);
            axisMeshComponent.primitive(0, 0).shadowsCastable(false);

            rotationToolEntity.addChild(axisEntity);
        }

        // @fixme There's an engine bug where doing that before adding children,
        // won't update children's transforms.
        gizmoEntity.get<sill::TransformComponent>().scaling(0.f);
        setGizmoTool(gameState, GizmoTool::Translation);
    }

    editorBehavior.onUpdate([&](float /* dt */) {
        if ((gameState.state == State::Idle || gameState.state == State::Editor) && engine.input().justDown("toggle-editor")) {
            gameState.state = (gameState.state == State::Editor) ? State::Idle : State::Editor;
            gameState.editor.state = EditorState::Idle;

            engine.physicsEngine().enabled(gameState.state != State::Editor);
            unselectAllEntities(gameState);

            if (gameState.state == State::Editor) setCameraMode(gameState, CameraMode::Orbit);
            else setCameraMode(gameState, CameraMode::FirstPerson);
        }

        if (gameState.state != State::Editor) return;

        if (gameState.editor.state == EditorState::Idle) {
            // ----- Gizmos

            float minToolAxisDistance = INFINITY;
            sill::GameEntity* selectedToolAxis = nullptr;
            for (auto toolAxis : gameState.editor.gizmo.toolEntity->children()) {
                float toolAxisDistance = toolAxis->distanceFrom(gameState.pickingRay);
                if (toolAxisDistance <= 0.f) continue;

                if (toolAxisDistance < minToolAxisDistance) {
                    minToolAxisDistance = toolAxisDistance;
                    selectedToolAxis = toolAxis;
                    break;
                }
            }

            // Display highlight for gizmos
            if (gameState.editor.gizmo.selectedToolAxis) {
                gameState.editor.gizmo.selectedToolAxis->get<sill::MeshComponent>().material(0, 0)->set("highlight", false);
            }
            gameState.editor.gizmo.selectedToolAxis = selectedToolAxis;
            if (selectedToolAxis) {
                selectedToolAxis->get<sill::MeshComponent>().material(0, 0)->set("highlight", true);
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
                gameState.level.bricks.emplace_back(std::move(brick));
                return;
            }
            else if (engine.input().justDown("add-barrier")) {
                auto barrier = std::make_unique<Barrier>(gameState);
                gameState.level.barriers.emplace_back(std::move(barrier));
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
                unselectAllEntities(gameState);
                return;
            }

            // ----- Saving

            if (engine.input().justDown("save")) {
                serializeLevel(gameState, gameState.level.path);
                return;
            }

            // ----- Left click

            // Go the gizmo mode if needed
            if (engine.input().justDown("left-fire")) {
                if (gameState.editor.gizmo.selectedToolAxis) {
                    uint32_t axisIndex = gameState.editor.gizmo.toolEntity->childIndex(*gameState.editor.gizmo.selectedToolAxis);
                    gameState.editor.gizmo.axis = g_axes[axisIndex];
                    gameState.editor.gizmo.nextAxis = g_axes[(axisIndex + 1u) % 3u];

                    Ray axisRay;
                    axisRay.origin = gameState.editor.gizmo.entity->get<sill::TransformComponent>().translation();
                    axisRay.direction = gameState.editor.gizmo.axis;

                    if (gameState.editor.gizmo.tool == GizmoTool::Translation) {
                        gameState.editor.state = EditorState::TranslateAlongAxis;
                        gameState.editor.gizmo.axisOffset = projectOn(axisRay, gameState.pickingRay);
                    }
                    else if (gameState.editor.gizmo.tool == GizmoTool::Rotation) {
                        gameState.editor.state = EditorState::RotateAlongAxis;
                        gameState.editor.gizmo.axisOffset = rotationAxisOffset(gameState, axisRay.origin);
                    }

                    return;
                }
            }

            // Select entity on left click if any.
            if (engine.input().justDownUp("left-fire")) {
                auto* pickedEntity = engine.pickEntity(gameState.pickingRay);
                selectEntity(gameState, pickedEntity, engine.input().down("editor.multiple-selection-modifier"));
            }
        }

        // ----- Selected entities control

        if (gameState.editor.selectedEntities.empty()) return;

        // Computing barycenter of selected entities, this is where the gizmo will be.
        glm::vec3 barycenter = glm::vec3{0.f};
        for (auto entity : gameState.editor.selectedEntities) {
            barycenter += entity->get<sill::TransformComponent>().translation();
        }
        barycenter /= gameState.editor.selectedEntities.size();

        gameState.editor.gizmo.entity->get<sill::TransformComponent>().translation(barycenter);
        const auto gizmoDistanceToCamera = std::sqrt(glm::length(gameState.camera.component->origin() - barycenter));
        gameState.editor.gizmo.entity->get<sill::TransformComponent>().scaling(gizmoDistanceToCamera);

        // Move entity if needed.
        if (gameState.editor.state == EditorState::TranslateAlongAxis) {
            if (engine.input().justUp("left-fire")) {
                gameState.editor.state = EditorState::Idle;
                return;
            }

            if (engine.input().axisChanged("main-x") || engine.input().axisChanged("main-y")) {
                const Ray axisRay = {.origin = barycenter, .direction = gameState.editor.gizmo.axis};
                const auto delta = projectOn(axisRay, gameState.pickingRay) - gameState.editor.gizmo.axisOffset;

                for (auto entity : gameState.editor.selectedEntities) {
                    entity->get<sill::TransformComponent>().translate(-delta * gameState.editor.gizmo.axis);
                }
            }
        }
        // Or rotate it!
        else if (gameState.editor.state == EditorState::RotateAlongAxis) {
            if (engine.input().justUp("left-fire")) {
                gameState.editor.state = EditorState::Idle;
                return;
            }

            if (engine.input().axisChanged("main-x") || engine.input().axisChanged("main-y")) {
                auto axisOffset = rotationAxisOffset(gameState, barycenter);
                auto delta = axisOffset - gameState.editor.gizmo.axisOffset;
                gameState.editor.gizmo.axisOffset = axisOffset;

                for (auto entity : gameState.editor.selectedEntities) {
                    entity->get<sill::TransformComponent>().rotateAround(gameState.editor.gizmo.axis, -delta, barycenter);
                }
            }
        }
        else if (gameState.editor.state == EditorState::Idle) {
            if (engine.input().justDownUp("editor.switch-gizmo")) {
                if (gameState.editor.gizmo.tool == GizmoTool::Translation) {
                    setGizmoTool(gameState, GizmoTool::Rotation);
                }
                else if (gameState.editor.gizmo.tool == GizmoTool::Rotation) {
                    setGizmoTool(gameState, GizmoTool::Translation);
                }
            }

            // If there is one barrier in the multiple selection,
            // we will be all panels and bricks to it.
            if (engine.input().justDown("bind-to-barrier")) {
                auto barriersCount = 0u;
                sill::GameEntity* barrierEntity = nullptr;
                for (auto entity : gameState.editor.selectedEntities) {
                    if (entity->name() == "barrier") {
                        barriersCount += 1u;
                        barrierEntity = entity;
                    }
                }

                if (barriersCount == 1u) {
                    // @todo Leaving this message as long as we don't have a way to visualize bindings
                    std::cout << "Binding all selected panels and bricks to selected barrier." << std::endl;

                    auto& barrier = *findBarrier(gameState, *barrierEntity);
                    for (auto entity : gameState.editor.selectedEntities) {
                        if (entity->name() == "brick") {
                            auto& brick = *findBrick(gameState, *entity);
                            brick.addBarrier(barrier);
                        }
                        else if (entity->name() == "panel") {
                            auto& panel = *findPanel(gameState, *entity);
                            panel.addBarrier(barrier);
                        }
                    }
                }
            }

            for (auto entity : gameState.editor.selectedEntities) {
                updateSelectedEntity(gameState, *entity);
            }
        }
    });
}
