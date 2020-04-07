#include "./editor.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>
#include <lava/chamber/math.hpp>
#include <lava/dike.hpp>
#include <lava/magma.hpp>

#include "./game-state.hpp"
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

void onSelectionChanged(GameState& gameState)
{
    // Erase UI
    for (auto entity : gameState.ui.entities) {
        gameState.engine->remove(*entity);
    }
    gameState.ui.entities.clear();

    // Create UI
    if (gameState.editor.selection.objects.size() == 1u) {
        auto& object = *gameState.editor.selection.objects[0u];
        if (object.entity().name() == "brick") {
            auto& brick = dynamic_cast<Brick&>(object);
            auto& entity = gameState.engine->make<sill::GameEntity>("button - toggle fixed");
            entity.make<sill::UiButtonComponent>(L"toggle fixed").onClicked([&brick]() {
                brick.fixed(!brick.fixed());
            });
            entity.get<sill::TransformComponent>().translation2d({100, 40});
            gameState.ui.entities.emplace_back(&entity);
            return;
        }
    }
}

void unselectAllObjects(GameState& gameState, bool signalSelectionChanged = true) {
    gameState.editor.selection.objects.clear();
    gameState.editor.gizmo.entity->get<sill::TransformComponent>().scaling(0.f);
    gameState.editor.selection.multiEntity->get<sill::TransformComponent>().scaling2d(0.f);

    if (signalSelectionChanged) {
        onSelectionChanged(gameState);
    }
}

void selectObject(GameState& gameState, Object* object, bool addToSelection, bool signalSelectionChanged = true) {
    if (!object) {
        if (!addToSelection) {
            unselectAllObjects(gameState, signalSelectionChanged);
        }
        return;
    }

    // If we already selected that object, ignore
    auto objectIt = std::find(gameState.editor.selection.objects.begin(), gameState.editor.selection.objects.end(), object);
    if (objectIt != gameState.editor.selection.objects.end()) {
        return;
    }

    if (!addToSelection) {
        unselectAllObjects(gameState, false);
    }

    gameState.editor.selection.objects.emplace_back(object);

    if (signalSelectionChanged) {
        onSelectionChanged(gameState);
    }
}

/// Update the the multi rectangle visuals.
void selectMultiObjectsUpdate(GameState& gameState)
{
    auto topLeft = glm::min(gameState.editor.selection.multiStart, gameState.editor.selection.multiEnd);
    auto bottomRight = glm::max(gameState.editor.selection.multiStart, gameState.editor.selection.multiEnd);
    gameState.editor.selection.multiEntity->get<sill::TransformComponent>().translation2d((topLeft + bottomRight) / 2.f);
    gameState.editor.selection.multiEntity->get<sill::TransformComponent>().scaling2d(bottomRight - topLeft);
    gameState.editor.selection.multiEntity->get<sill::FlatComponent>().primitive(0u, 0u).material()->set("extent", bottomRight - topLeft);
}

/// Update the current selection based on the multi rectangle.
void selectMultiObjects(GameState& gameState, bool signalSelectionChanged = true)
{
    unselectAllObjects(gameState, false);

    auto topLeft = glm::min(gameState.editor.selection.multiStart, gameState.editor.selection.multiEnd);
    auto bottomRight = glm::max(gameState.editor.selection.multiStart, gameState.editor.selection.multiEnd);
    auto frustum = gameState.camera.component->frustum(topLeft, bottomRight);

    for (auto object : gameState.level.objects) {
        auto position = object->transform().translation();
        if (frustum.canSee(position)) {
            selectObject(gameState, object, true, false);
        }
    }

    // @todo Well, not sure selection changed...
    if (signalSelectionChanged) {
        onSelectionChanged(gameState);
    }
}

// Specific behavior based on what it selected.
void updateSelectedObject(GameState& gameState, Object& object)
{
    auto& input = gameState.engine->input();
    auto& entity = object.entity();

    if (gameState.editor.selection.objects.size() != 1u) return;

    if (entity.name() == "brick") {
        auto& brick = dynamic_cast<Brick&>(object);

        if (input.justDown("up")) {
            brick.addBlockV(0, true);
        }
        if (input.justDown("down")) {
            brick.addBlockV(0, false);
        }
        if (input.justDown("right")) {
            brick.addBlockH(0, true);
        }
        if (input.justDown("left")) {
            brick.addBlockH(0, false);
        }
    }
    else if (entity.name() == "panel") {
        auto& panel = dynamic_cast<Panel&>(object);

        if (input.justDown("up")) {
            panel.extent(panel.extent() + glm::uvec2(0, 1));
        }
        if (input.justDown("down")) {
            panel.extent(panel.extent() - glm::uvec2(0, 1));
        }
        if (input.justDown("right")) {
            panel.extent(panel.extent() + glm::uvec2(1, 0));
        }
        if (input.justDown("left")) {
            panel.extent(panel.extent() - glm::uvec2(1, 0));
        }
        if (input.justDown("rename-selection")) {
            std::string panelName;
            std::cout << "Renaming panel '" << panel.name() << "'. New name:" << std::endl;
            std::cin >> panelName;
            panel.name(panelName);
            std::cout << "New panel name '" << panel.name() << "'." << std::endl;
        }
        if (input.justDown("solve-selection")) {
            panel.pretendSolved(!panel.solved());
        }
    }
    else if (entity.name() == "barrier") {
        auto& barrier = dynamic_cast<Barrier&>(object);

        if (input.justDown("up")) {
            barrier.diameter(barrier.diameter() + 0.5f);
        }
        if (input.justDown("down")) {
            barrier.diameter(barrier.diameter() - 0.5f);
        }
        if (input.justDown("rename-selection")) {
            std::string barrierName;
            std::cout << "Renaming barrier '" << barrier.name() << "'. New name:" << std::endl;
            std::cin >> barrierName;
            barrier.name(barrierName);
            std::cout << "New barrier name '" << barrier.name() << "'." << std::endl;
        }
        if (input.justDown("solve-selection")) {
            barrier.powered(!barrier.powered());
        }
    }
    else {
        // Nothing to do yet.
        // auto& generic = dynamic_cast<Generic&>(object);
    }
}

void setupEditor(GameState& gameState)
{
    auto& engine = *gameState.engine;
    auto& input = engine.input();

    // Inputs

    input.bindAction("editor.toggle", Key::E);
    input.bindAction("editor.exit", Key::Escape);
    input.bindAction("editor.switch-gizmo", MouseButton::Middle);
    input.bindAction("editor.multiple-selection-modifier", {Key::LeftShift});
    input.bindAction("editor.multiple-selection-modifier", {Key::RightShift});
    input.bindAction("editor.duplicate-selection", {Key::LeftControl, Key::D});
    input.bindAction("editor.delete-selection", {Key::Delete});

    input.bindAction("up", Key::Up);
    input.bindAction("down", Key::Down);
    input.bindAction("left", Key::Left);
    input.bindAction("right", Key::Right);
    input.bindAction("save", {Key::LeftControl, Key::S});
    input.bindAction("reload-level", {Key::LeftControl, Key::R});

    // @fixme This is for debug for now
    input.bindAction("add-panel", {Key::LeftShift, Key::A, Key::P});
    input.bindAction("add-brick", {Key::LeftShift, Key::A, Key::B});
    input.bindAction("add-barrier", {Key::LeftShift, Key::A, Key::R});
    input.bindAction("add-mesh", {Key::LeftShift, Key::A, Key::M});
    input.bindAction("bind-to-barrier", {Key::LeftAlt, Key::R});
    input.bindAction("rename-selection", {Key::F2});
    input.bindAction("solve-selection", {Key::LeftShift, Key::S});
    // @fixme Disabling, need gizmo
    // input.bindAction("scale-up", Key::S);
    // input.bindAction("scale-down", {Key::LeftShift, Key::S});
    // @todo Make action to switch to rotation gizmo on R

    auto& editorEntity = engine.make<sill::GameEntity>("editor");
    auto& editorBehavior = editorEntity.make<sill::BehaviorComponent>();

    // Multi rectangle
    {
        auto& multiEntity = engine.make<sill::GameEntity>("multi");
        auto& flatComponent = multiEntity.make<sill::FlatComponent>();
        auto& flatNode = sill::makers::quadFlatMaker(1.f)(flatComponent);
        auto& material = engine.scene2d().make<magma::Material>("selection-rectangle");
        flatNode.flatGroup->primitive(0u).material(material);
        gameState.editor.selection.multiEntity = &multiEntity;
    }

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
        if (((gameState.state == State::Idle || gameState.state == State::Editor) && input.justDown("editor.toggle")) ||
            ((gameState.state == State::Editor) && input.justDown("editor.exit"))) {
            gameState.state = (gameState.state == State::Editor) ? State::Idle : State::Editor;
            gameState.editor.state = EditorState::Idle;

            engine.physicsEngine().enabled(gameState.state != State::Editor);
            unselectAllObjects(gameState);

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

            if (input.justDown("add-panel")) {
                auto panel = std::make_unique<Panel>(gameState);
                panel->extent({3, 3});
                gameState.level.panels.emplace_back(std::move(panel));
                return;
            }
            else if (input.justDown("add-brick")) {
                auto brick = std::make_unique<Brick>(gameState);
                brick->blocks({{0, 0}});
                gameState.level.bricks.emplace_back(std::move(brick));
                return;
            }
            else if (input.justDown("add-barrier")) {
                auto barrier = std::make_unique<Barrier>(gameState);
                gameState.level.barriers.emplace_back(std::move(barrier));
                return;
            }
            else if (input.justDown("add-mesh")) {
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

                auto generic = std::make_unique<Generic>(gameState);
                generic->entity(entity);
                gameState.level.generics.emplace_back(std::move(generic));
                return;
            }

            // ----- Reload

            if (input.justDown("reload-level")) {
                unselectAllObjects(gameState);
                loadLevel(gameState, gameState.level.path);
                return;
            }

            // ----- Saving

            if (input.justDown("save")) {
                serializeLevel(gameState, gameState.level.path);
                return;
            }

            // ----- Left click

            // Go the gizmo mode if needed
            if (input.justDown("left-fire")) {
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
            // Or might be multi-selection mode
            else if (input.down("left-fire") &&
                     (input.axisChanged("main-x") || input.axisChanged("main-y"))) {
                gameState.editor.state = EditorState::MultiSelection;
                gameState.editor.selection.multiStart = input.mouseCoordinates();
                unselectAllObjects(gameState);
            }

            // Select entity on left click if any.
            if (input.justDownUp("left-fire")) {
                auto pickedEntity = engine.pickEntity(gameState.pickingRay);
                auto pickedObject = findObject(gameState, pickedEntity);
                selectObject(gameState, pickedObject, input.down("editor.multiple-selection-modifier"));
            }
        }
        else if (gameState.editor.state == EditorState::MultiSelection) {
            if (input.justUp("left-fire")) {
                gameState.editor.state = EditorState::Idle;
                selectMultiObjects(gameState);
            }
            else if (input.axisChanged("main-x") || input.axisChanged("main-y")) {
                gameState.editor.selection.multiEnd = input.mouseCoordinates();
                selectMultiObjectsUpdate(gameState);
            }
        }

        // ----- Selected objects control

        if (gameState.editor.selection.objects.empty()) return;

        // Computing barycenter of selected objects, this is where the gizmo will be.
        glm::vec3 barycenter = glm::vec3{0.f};
        for (auto object : gameState.editor.selection.objects) {
            barycenter += object->transform().translation();
        }
        barycenter /= gameState.editor.selection.objects.size();

        gameState.editor.gizmo.entity->get<sill::TransformComponent>().translation(barycenter);
        const auto gizmoDistanceToCamera = std::sqrt(glm::length(gameState.camera.component->origin() - barycenter));
        gameState.editor.gizmo.entity->get<sill::TransformComponent>().scaling(gizmoDistanceToCamera);

        // Move object if needed.
        if (gameState.editor.state == EditorState::TranslateAlongAxis) {
            if (input.justUp("left-fire")) {
                gameState.editor.state = EditorState::Idle;
                return;
            }

            if (input.axisChanged("main-x") || input.axisChanged("main-y")) {
                const Ray axisRay = {.origin = barycenter, .direction = gameState.editor.gizmo.axis};
                const auto delta = projectOn(axisRay, gameState.pickingRay) - gameState.editor.gizmo.axisOffset;

                for (auto object : gameState.editor.selection.objects) {
                    object->transform().translate(-delta * gameState.editor.gizmo.axis);
                }
            }
        }
        // Or rotate it!
        else if (gameState.editor.state == EditorState::RotateAlongAxis) {
            if (input.justUp("left-fire")) {
                gameState.editor.state = EditorState::Idle;
                return;
            }

            if (input.axisChanged("main-x") || input.axisChanged("main-y")) {
                auto axisOffset = rotationAxisOffset(gameState, barycenter);
                auto delta = axisOffset - gameState.editor.gizmo.axisOffset;
                gameState.editor.gizmo.axisOffset = axisOffset;

                for (auto object : gameState.editor.selection.objects) {
                    object->transform().rotateAround(gameState.editor.gizmo.axis, -delta, barycenter);
                }
            }
        }
        else if (gameState.editor.state == EditorState::Idle) {
            if (input.justDownUp("editor.switch-gizmo")) {
                if (gameState.editor.gizmo.tool == GizmoTool::Translation) {
                    setGizmoTool(gameState, GizmoTool::Rotation);
                }
                else if (gameState.editor.gizmo.tool == GizmoTool::Rotation) {
                    setGizmoTool(gameState, GizmoTool::Translation);
                }
            }

            // If there is one barrier in the multiple selection,
            // we will be all panels and bricks to it.
            if (input.justDown("bind-to-barrier")) {
                auto barriersCount = 0u;
                Object* barrierObject = nullptr;
                for (auto object : gameState.editor.selection.objects) {
                    if (object->entity().name() == "barrier") {
                        barriersCount += 1u;
                        barrierObject = object;
                    }
                }

                if (barriersCount == 1u) {
                    // @todo Leaving this message as long as we don't have a way to visualize bindings
                    std::cout << "Binding all selected panels and bricks to selected barrier." << std::endl;

                    auto& barrier = dynamic_cast<Barrier&>(*barrierObject);
                    for (auto object : gameState.editor.selection.objects) {
                        if (object->entity().name() == "brick") {
                            auto& brick = dynamic_cast<Brick&>(*object);
                            brick.addBarrier(barrier);
                        }
                        else if (object->entity().name() == "panel") {
                            auto& panel = dynamic_cast<Panel&>(*object);
                            panel.addBarrier(barrier);
                        }
                    }
                }
            }

            // Deleting all selected objects.
            if (input.justDown("editor.delete-selection")) {
                for (auto object : gameState.editor.selection.objects) {
                    object->clear();
                }
                unselectAllObjects(gameState);
                return;
            }
            // Duplicate all selected objects.
            else if (input.justDown("editor.duplicate-selection")) {
                // Have them all move as the span of all objects.
                auto minX = 0.f;
                auto maxX = 0.f;
                for (auto object : gameState.editor.selection.objects) {
                    auto origin = object->transform().translation().x - barycenter.x;
                    auto halfSpan = object->halfSpan();
                    minX = std::min(minX, origin - halfSpan);
                    maxX = std::max(maxX, origin + halfSpan);
                }
                auto offset = maxX - minX;

                for (auto object : gameState.editor.selection.objects) {
                    auto& newObject = duplicateBySerialization(gameState, *object);
                    newObject.transform().translate(glm::vec3{offset, 0.f, 0.f});
                }
            }

            for (auto object : gameState.editor.selection.objects) {
                updateSelectedObject(gameState, *object);
            }
        }
    });
}
