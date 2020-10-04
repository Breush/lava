#include "./editor.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <lava/chamber/math.hpp>
#include <lava/chamber/string-tools.hpp>
#include <lava/dike.hpp>

#include "./game-state.hpp"
#include "./camera.hpp"
#include "./environment.hpp"
#include "./objects/pedestal.hpp"
#include "./serializer.hpp"

using namespace lava;
using namespace lava::chamber;

namespace {
    std::array<glm::vec3, 3u> g_axes = {glm::vec3{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

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
        gameState.editor.gizmo.scalingToolEntity->get<sill::TransformComponent>().scaling(0.f);
    }
    else if (gameState.editor.gizmo.tool == GizmoTool::Rotation) {
        gameState.editor.gizmo.toolEntity = gameState.editor.gizmo.rotationToolEntity;
        gameState.editor.gizmo.translationToolEntity->get<sill::TransformComponent>().scaling(0.f);
        gameState.editor.gizmo.rotationToolEntity->get<sill::TransformComponent>().scaling(1.f);
        gameState.editor.gizmo.scalingToolEntity->get<sill::TransformComponent>().scaling(0.f);
    }
    else if (gameState.editor.gizmo.tool == GizmoTool::Scaling) {
        gameState.editor.gizmo.toolEntity = gameState.editor.gizmo.scalingToolEntity;
        gameState.editor.gizmo.translationToolEntity->get<sill::TransformComponent>().scaling(0.f);
        gameState.editor.gizmo.rotationToolEntity->get<sill::TransformComponent>().scaling(0.f);
        gameState.editor.gizmo.scalingToolEntity->get<sill::TransformComponent>().scaling(1.f);
    }
}

void reflowUi(GameState& gameState)
{
    constexpr float xMargin = 4.f;
    constexpr float yMargin = 4.f;
    float y = yMargin;

    for (const auto& widget : gameState.editor.ui.widgets) {
        auto kind = widget.kind();
        auto& entity = *widget.entity();
        if (kind == UiWidgetKind::TextEntry) {
            auto& uiComponent = entity.get<sill::UiTextEntryComponent>();
            auto& extent = uiComponent.extent();
            entity.get<sill::TransformComponent>().translation2d({xMargin + extent.x / 2.f, y + extent.y / 2.f});
            y += yMargin + extent.y;
        }
        else if (kind == UiWidgetKind::ToggleButton) {
            auto& uiComponent = entity.get<sill::UiButtonComponent>();
            auto& extent = uiComponent.extent();
            entity.get<sill::TransformComponent>().translation2d({xMargin + extent.x / 2.f, y + extent.y / 2.f});
            y += yMargin + extent.y;
        }
        else if (kind == UiWidgetKind::Select) {
            auto& uiComponent = entity.get<sill::UiSelectComponent>();
            auto& extent = uiComponent.extent();
            entity.get<sill::TransformComponent>().translation2d({xMargin + extent.x / 2.f, y + extent.y / 2.f});
            y += yMargin + extent.y;
        }
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

        gameState.editor.ui.widgets.clear();
        object.uiWidgets(gameState.editor.ui.widgets);

        for (auto& widget : gameState.editor.ui.widgets) {
            auto kind = widget.kind();
            auto& entity = gameState.engine->make<sill::Entity>("ui." + widget.id());
            if (kind == UiWidgetKind::TextEntry) {
                auto& textEntry = widget.textEntry();
                auto& uiComponent = entity.make<sill::UiTextEntryComponent>(utf8to16(textEntry.getter()));
                uiComponent.onTextChanged([textEntry](const std::wstring& text) {
                    textEntry.setter(utf16to8(text));
                });
            }
            else if (kind == UiWidgetKind::ToggleButton) {
                auto& toggleButton = widget.toggleButton();
                auto& uiComponent = entity.make<sill::UiButtonComponent>(utf8to16(toggleButton.text + ": " + std::to_string(toggleButton.getter())));
                uiComponent.onClicked([toggleButton, &uiComponent]() {
                    toggleButton.setter(!toggleButton.getter());
                    uiComponent.text(utf8to16(toggleButton.text + ": " + std::to_string(toggleButton.getter())));
                });
                uiComponent.onExtentChanged([&gameState](const glm::vec2& /* extent */) {
                    reflowUi(gameState);
                });
            }
            else if (kind == UiWidgetKind::Select) {
                auto& select = widget.select();
                std::vector<std::wstring> options;
                options.reserve(select.options.size());
                for (auto option : select.options) {
                    options.emplace_back(utf8to16(std::string(option)));
                }
                auto& uiComponent = entity.make<sill::UiSelectComponent>(options, select.getter());
                uiComponent.onIndexChanged([select](uint8_t index, const std::wstring&) {
                    select.setter(index);
                });
            }
            gameState.ui.entities.emplace_back(&entity);
            widget.entity(&entity);
        }

        reflowUi(gameState);
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

    auto& flatComponent = gameState.editor.selection.multiEntity->get<sill::FlatComponent>();
    gameState.editor.selection.multiEntity->get<sill::TransformComponent>().translation2d((topLeft + bottomRight) / 2.f);
    gameState.editor.selection.multiEntity->get<sill::TransformComponent>().scaling2d(1.f);
    flatComponent.node(0u).transform(glm::scale(glm::mat3(1.f), bottomRight - topLeft));
    flatComponent.material(0u, 0u)->set("extent", bottomRight - topLeft);
}

/// Update the current selection based on the multi rectangle.
void selectMultiObjects(GameState& gameState, bool signalSelectionChanged = true)
{
    unselectAllObjects(gameState, false);

    auto topLeft = glm::min(gameState.editor.selection.multiStart, gameState.editor.selection.multiEnd);
    auto bottomRight = glm::max(gameState.editor.selection.multiStart, gameState.editor.selection.multiEnd);
    auto frustum = gameState.camera.component->unprojectAsFrustum(topLeft, bottomRight);

    for (const auto& object : gameState.level.objects) {
        const auto& position = object->editorOrigin();
        if (frustum.canSee(position)) {
            selectObject(gameState, object.get(), true, false);
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
    if (gameState.editor.selection.objects.size() != 1u) return;

    auto& input = gameState.engine->input();
    const auto& kind = object.kind();

    if (kind == "brick") {
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
    else if (kind == "panel") {
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
    }
}

void setupEditor(GameState& gameState)
{
    auto& engine = *gameState.engine;
    auto& input = engine.input();

    gameState.editor.resources.colliderMaterial = engine.scene().makeMaterial("collider");

    // Inputs
    input.bindAction("editor.toggle", Key::E);
    input.bindAction("editor.exit", Key::Escape);
    input.bindAction("editor.main-click", MouseButton::Left);
    input.bindAction("editor.switch-gizmo", MouseButton::Middle);
    input.bindAction("editor.multiple-selection-modifier", {Key::LeftShift});
    input.bindAction("editor.multiple-selection-modifier", {Key::RightShift});
    input.bindAction("editor.duplicate-selection", {Key::LeftControl, Key::D});
    input.bindAction("editor.delete-selection", {Key::Delete});

    input.bindAxis("editor.axis-x", InputAxis::MouseX);
    input.bindAxis("editor.axis-y", InputAxis::MouseY);

    input.bindAction("up", Key::Up);
    input.bindAction("down", Key::Down);
    input.bindAction("left", Key::Left);
    input.bindAction("right", Key::Right);
    input.bindAction("save", {Key::LeftControl, Key::S});
    input.bindAction("reload-level", {Key::LeftControl, Key::R});

    // @fixme This is for debug for now, we want UI
    input.bindAction("add-panel", {Key::LeftShift, Key::A, Key::P});
    input.bindAction("add-brick", {Key::LeftShift, Key::A, Key::B});
    input.bindAction("add-barrier", {Key::LeftShift, Key::A, Key::R});
    input.bindAction("add-pedestal", {Key::LeftShift, Key::A, Key::D});
    input.bindAction("add-mesh", {Key::LeftShift, Key::A, Key::M});
    input.bindAction("add-object", {Key::LeftShift, Key::A, Key::O});
    input.bindAction("bind-to-pedestal", {Key::LeftAlt, Key::D});
    // @todo Make action to switch to rotation gizmo on R

    auto& editorEntity = engine.make<sill::Entity>("editor");
    auto& editorBehavior = editorEntity.make<sill::BehaviorComponent>();

    // Multi rectangle
    {
        auto& multiEntity = engine.make<sill::Entity>("multi");
        auto& flatComponent = multiEntity.make<sill::FlatComponent>();
        auto& flatNode = sill::makers::quadFlatMaker(1.f)(flatComponent);
        auto material = engine.scene2d().makeMaterial("selection-rectangle");
        flatNode.flatGroup->primitive(0u).material(material);
        gameState.editor.selection.multiEntity = &multiEntity;
    }

    // Gizmo
    {
        auto sceneIndex = engine.addScene();
        gameState.editor.gizmo.scene = &engine.scene(sceneIndex);
        gameState.editor.gizmo.camera = &engine.scene(sceneIndex).make<magma::Camera>(engine.windowRenderTarget().extent());

        // @fixme We need at least a light, otherwise nothing works!
        auto& light = engine.scene(sceneIndex).make<magma::Light>();
        light.shadowsEnabled(false);

        // Auto-update camera extent
        gameState.engine->onWindowExtentChanged([&gameState](const Extent2d& extent) {
            gameState.editor.gizmo.camera->extent(extent);
        });

        Viewport viewport;
        viewport.depth = -0.5f; // On top of main scene, but below UI.
        engine.renderEngine().addView(gameState.editor.gizmo.camera->renderImage(), engine.windowRenderTarget(), viewport);

        auto& gizmoEntity = engine.make<sill::Entity>("gizmo");
        gizmoEntity.ensure<sill::TransformComponent>().scaling(0.f);
        gameState.editor.gizmo.entity = &gizmoEntity;

        // --- Translation
        auto& translationToolEntity = engine.make<sill::Entity>("gizmo-translation");
        translationToolEntity.ensure<sill::TransformComponent>();
        gizmoEntity.addChild(translationToolEntity);
        gameState.editor.gizmo.translationToolEntity = &translationToolEntity;

        for (const auto& axis : g_axes) {
            auto& axisEntity = engine.make<sill::Entity>("gizmo-translation-axis");
            auto axisMaterial = engine.scene(sceneIndex).makeMaterial("gizmo");
            axisMaterial->set("color", axis);

            auto& axisMeshComponent = axisEntity.make<sill::MeshComponent>(sceneIndex);
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
        auto& rotationToolEntity = engine.make<sill::Entity>("gizmo-rotation");
        rotationToolEntity.ensure<sill::TransformComponent>();
        gizmoEntity.addChild(rotationToolEntity);
        gameState.editor.gizmo.rotationToolEntity = &rotationToolEntity;

        for (const auto& axis : g_axes) {
            auto& axisEntity = engine.make<sill::Entity>("gizmo-rotation-axis");
            auto axisMaterial = engine.scene(sceneIndex).makeMaterial("gizmo");
            axisMaterial->set("color", axis);

            auto& axisMeshComponent = axisEntity.make<sill::MeshComponent>(sceneIndex);
            auto matrix = glm::rotate(glm::mat4(1.f), math::PI_OVER_TWO, {0, 0, 1});
            matrix = glm::rotate(matrix, math::PI_OVER_TWO, axis);
            auto nodeIndex = sill::makers::toreMeshMaker(32u, 1.f, 4u, 0.02f)(axisMeshComponent);
            axisMeshComponent.nodeMatrix(nodeIndex, matrix); // @todo Have a generic way to bake matrix to vertices
            axisMeshComponent.primitive(nodeIndex, 0).material(axisMaterial);
            axisMeshComponent.primitive(nodeIndex, 0).shadowsCastable(false);

            rotationToolEntity.addChild(axisEntity);
        }

        // --- Scaling
        auto& scalingToolEntity = engine.make<sill::Entity>("gizmo-scaling");
        scalingToolEntity.ensure<sill::TransformComponent>();
        gizmoEntity.addChild(scalingToolEntity);
        gameState.editor.gizmo.scalingToolEntity = &scalingToolEntity;

        for (const auto& axis : g_axes) {
            auto& axisEntity = engine.make<sill::Entity>("gizmo-scaling-axis");
            auto axisMaterial = engine.scene(sceneIndex).makeMaterial("gizmo");
            axisMaterial->set("color", axis);

            auto transform = glm::rotate(glm::mat4(1.f), math::PI_OVER_TWO, {0, 0, 1});
            transform = glm::rotate(transform, math::PI_OVER_TWO, axis);

            auto& axisMeshComponent = axisEntity.make<sill::MeshComponent>(sceneIndex);
            sill::makers::CylinderMeshOptions cylinderOptions;
            cylinderOptions.transform = transform;
            cylinderOptions.offset = 0.25f;
            sill::makers::cylinderMeshMaker(8u, 0.02f, 0.5f, cylinderOptions)(axisMeshComponent);
            axisMeshComponent.primitive(0, 0).material(axisMaterial);
            axisMeshComponent.primitive(0, 0).shadowsCastable(false);

            sill::makers::BoxMeshOptions boxOptions;
            boxOptions.offset = glm::vec3(transform * glm::vec4{0.f, 0.f, 0.5f, 1.f});
            sill::makers::boxMeshMaker(0.05f, boxOptions)(axisMeshComponent);
            axisMeshComponent.primitive(1, 0).material(axisMaterial);
            axisMeshComponent.primitive(1, 0).shadowsCastable(false);

            scalingToolEntity.addChild(axisEntity);
        }

        setGizmoTool(gameState, GizmoTool::Translation);
    }

    editorBehavior.onUpdate([&](float /* dt */) {
        if (((gameState.state == State::Idle || gameState.state == State::Editor) && input.justDown("editor.toggle")) ||
            ((gameState.state == State::Editor) && input.justDown("editor.exit"))) {
            gameState.state = (gameState.state == State::Editor) ? State::Idle : State::Editor;
            gameState.editor.state = EditorState::Idle;

            engine.physicsEngine().enabled(gameState.state != State::Editor);
            unselectAllObjects(gameState);

            setCameraMode(gameState, (gameState.state == State::Editor) ? CameraMode::Orbit : CameraMode::FirstPerson);
        }

        if (gameState.state != State::Editor) return;

        // Synchronize the overlay camera with the main one.
        gameState.editor.gizmo.camera->viewTransform(gameState.camera.component->camera().viewTransform());
        gameState.editor.gizmo.camera->projectionMatrix(gameState.camera.component->camera().projectionMatrix());

        if (gameState.editor.state == EditorState::Idle) {
            // ----- Gizmos

            float minToolAxisDistance = INFINITY;
            sill::Entity* selectedToolAxis = nullptr;
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

            if (input.justDown("add-mesh")) {
                std::cout << "Available meshes:" << std::endl;
                for (const auto& entry : std::filesystem::recursive_directory_iterator("./assets/models/vr-puzzle/")) {
                    if (entry.path().extension() == ".glb") {
                        std::cout << entry.path() << std::endl;
                    }
                }

                std::string fileName;
                std::cout << "Please enter a .glb file name (without path nor extension):" << std::endl;
                std::cin >> fileName;
                auto filePath = "./assets/models/vr-puzzle/" + fileName + ".glb";

                auto& generic = Object::make(gameState, "generic");
                auto& entity = generic.entity();
                entity.name(fileName);
                auto& meshComponent = entity.make<sill::MeshComponent>();
                sill::makers::glbMeshMaker(filePath)(meshComponent);

                lava::Transform transform;
                transform.translation = gameState.camera.component->origin();
                generic.consolidateReferences();
                generic.transform().worldTransform(transform);
                return;
            }

            if (input.justDown("add-object")) {
                auto& generic = Object::make(gameState, "generic");
                auto& entity = generic.entity();
                entity.ensure<sill::TransformComponent>();

                lava::Transform transform;
                transform.translation = gameState.camera.component->origin();
                generic.transform().worldTransform(transform);

                selectObject(gameState, &generic, false);
            }

            std::string kindToAdd;
            if (input.justDown("add-barrier")) {
                kindToAdd = "barrier";
            }
            else if (input.justDown("add-brick")) {
                kindToAdd = "brick";
            }
            else if (input.justDown("add-panel")) {
                kindToAdd = "panel";
            }
            else if (input.justDown("add-pedestal")) {
                kindToAdd = "pedestal";
            }

            if (!kindToAdd.empty()) {
                auto& generic = Generic::make(gameState, kindToAdd);
                lava::Transform transform;
                transform.translation = gameState.camera.component->origin();
                generic.consolidateReferences();
                generic.transform().worldTransform(transform);
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
            if (input.justDown("editor.main-click")) {
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
                    else if (gameState.editor.gizmo.tool == GizmoTool::Scaling) {
                        gameState.editor.state = EditorState::ScaleAlongAxis;
                        gameState.editor.gizmo.axisOffset = projectOn(axisRay, gameState.pickingRay);
                        gameState.editor.gizmo.previousScaling = 1.f;
                    }

                    return;
                }
            }
            // Or might be multi-selection mode
            else if (input.down("editor.main-click") &&
                     (input.axisChanged("editor.axis-x") || input.axisChanged("editor.axis-y"))) {
                gameState.editor.state = EditorState::MultiSelection;
                gameState.editor.selection.multiStart = input.mouseCoordinates();
                unselectAllObjects(gameState);
            }

            // Select entity on left click if any.
            if (input.justDownUp("editor.main-click")) {
                float distance = 0.f;
                auto pickedEntity = engine.pickEntity(gameState.pickingRay, sill::PickPrecision::Mesh, &distance);
                auto pickedObject = findObject(gameState, pickedEntity);
                selectObject(gameState, pickedObject, input.down("editor.multiple-selection-modifier"));
                if (pickedObject) {
                    pickedObject->editorOnClicked(gameState.pickingRay.origin + distance * gameState.pickingRay.direction);
                }
            }
        }
        else if (gameState.editor.state == EditorState::MultiSelection) {
            if (input.justUp("editor.main-click")) {
                gameState.editor.state = EditorState::Idle;
                selectMultiObjects(gameState);
            }
            else if (input.axisChanged("editor.axis-x") || input.axisChanged("editor.axis-y")) {
                gameState.editor.selection.multiEnd = input.mouseCoordinates();
                selectMultiObjectsUpdate(gameState);
            }
        }

        // ----- Selected objects control

        if (gameState.editor.selection.objects.empty()) return;

        // Computing barycenter of selected objects, this is where the gizmo will be.
        glm::vec3 barycenter = glm::vec3{0.f};
        for (auto object : gameState.editor.selection.objects) {
            barycenter += object->editorOrigin();
        }
        barycenter /= gameState.editor.selection.objects.size();

        gameState.editor.gizmo.entity->get<sill::TransformComponent>().translation(barycenter);
        const auto gizmoDistanceToCamera = std::sqrt(glm::length(gameState.camera.component->origin() - barycenter));
        gameState.editor.gizmo.entity->get<sill::TransformComponent>().scaling(gizmoDistanceToCamera);

        // Move object if needed.
        if (gameState.editor.state == EditorState::TranslateAlongAxis) {
            if (input.justUp("editor.main-click")) {
                gameState.editor.state = EditorState::Idle;
                return;
            }

            if (input.axisChanged("editor.axis-x") || input.axisChanged("editor.axis-y")) {
                const Ray axisRay = {.origin = barycenter, .direction = gameState.editor.gizmo.axis};
                const auto delta = projectOn(axisRay, gameState.pickingRay) - gameState.editor.gizmo.axisOffset;

                for (auto object : gameState.editor.selection.objects) {
                    object->editorTranslate(-delta * gameState.editor.gizmo.axis);
                }
            }
        }
        // Or rotate it!
        else if (gameState.editor.state == EditorState::RotateAlongAxis) {
            if (input.justUp("editor.main-click")) {
                gameState.editor.state = EditorState::Idle;
                return;
            }

            if (input.axisChanged("editor.axis-x") || input.axisChanged("editor.axis-y")) {
                auto axisOffset = rotationAxisOffset(gameState, barycenter);
                auto delta = axisOffset - gameState.editor.gizmo.axisOffset;
                gameState.editor.gizmo.axisOffset = axisOffset;

                for (auto object : gameState.editor.selection.objects) {
                    object->transform().worldRotateFrom(gameState.editor.gizmo.axis, -delta, barycenter);
                }
            }
        }
        // Or scale it!!
        else if (gameState.editor.state == EditorState::ScaleAlongAxis) {
            if (input.justUp("editor.main-click")) {
                gameState.editor.state = EditorState::Idle;
                return;
            }

            if (input.axisChanged("editor.axis-x") || input.axisChanged("editor.axis-y")) {
                const Ray axisRay = {.origin = barycenter, .direction = gameState.editor.gizmo.axis};
                auto newScaling = projectOn(axisRay, gameState.pickingRay) / gameState.editor.gizmo.axisOffset;
                auto scaleFactor = newScaling / gameState.editor.gizmo.previousScaling;
                gameState.editor.gizmo.previousScaling = newScaling;

                for (auto object : gameState.editor.selection.objects) {
                    object->transform().worldScaleFrom(scaleFactor, barycenter);
                }
            }
        }
        else if (gameState.editor.state == EditorState::Idle) {
            if (input.justDownUp("editor.switch-gizmo")) {
                if (gameState.editor.gizmo.tool == GizmoTool::Translation) {
                    setGizmoTool(gameState, GizmoTool::Rotation);
                }
                else if (gameState.editor.gizmo.tool == GizmoTool::Rotation) {
                    setGizmoTool(gameState, GizmoTool::Scaling);
                }
                else if (gameState.editor.gizmo.tool == GizmoTool::Scaling) {
                    setGizmoTool(gameState, GizmoTool::Translation);
                }
            }

            // If there is one pedestal in the multiple selection,
            // we will bind all bricks to it.
            if (input.justDown("bind-to-pedestal")) {
                auto pedestalCount = 0u;
                Pedestal* pedestal = nullptr;
                for (auto object : gameState.editor.selection.objects) {
                    if (object->kind() == "pedestal") {
                        pedestalCount += 1u;
                        pedestal = dynamic_cast<Pedestal*>(object);
                    }
                }

                if (pedestalCount == 1u) {
                    for (auto object : gameState.editor.selection.objects) {
                        if (object->kind() == "brick") {
                            auto& brick = dynamic_cast<Brick&>(*object);
                            pedestal->addBrick(brick);
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
                    auto origin = object->transform().worldTransform().translation.x - barycenter.x;
                    auto halfSpan = object->halfSpan();
                    minX = std::min(minX, origin - halfSpan);
                    maxX = std::max(maxX, origin + halfSpan);
                }
                auto offset = maxX - minX;

                std::vector<Object*> newObjects;
                for (auto object : gameState.editor.selection.objects) {
                    auto& newObject = duplicateBySerialization(gameState, *object);
                    newObject.transform().worldTranslate(glm::vec3{offset, 0.f, 0.f});
                    newObjects.emplace_back(&newObject);
                }

                selectObject(gameState, nullptr, false);
                for (auto newObject : newObjects) {
                    selectObject(gameState, newObject, true);
                }
            }

            for (auto object : gameState.editor.selection.objects) {
                updateSelectedObject(gameState, *object);
            }
        }
    });
}
