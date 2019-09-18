#include "./editor.hpp"

#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <lava/chamber/math.hpp>
#include <lava/dike.hpp>
#include <lava/magma.hpp>

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

    engine.input().bindAction("save", {Key::LeftControl, Key::S});

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
            axisMeshComponent.node(0).mesh->primitive(0).material(axisMaterial);
            axisMeshComponent.node(0).mesh->primitive(0).shadowsCastable(false);

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
            if (engine.input().justDown("left-fire")) {
                // Check for gizmo hit (above all other entities).
                for (auto gizmoChild : gameState.editor.gizmoEntity->children()) {
                    if (gizmoChild->distanceFrom(gameState.pickingRay) > 0.f && gizmoChild->name() == "gizmo-axis") {
                        uint32_t axisIndex = gameState.editor.gizmoEntity->childIndex(*gizmoChild);
                        gameState.editor.state = EditorState::MoveAlongAxis;
                        gameState.editor.axis = g_axes[axisIndex];

                        Ray axisRay;
                        axisRay.origin = gameState.editor.selectedEntity->get<sill::TransformComponent>().translation();
                        axisRay.direction = gameState.editor.axis;
                        gameState.editor.axisOffset = projectOn(axisRay, gameState.pickingRay);
                        return;
                    }
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
            }
        }

        if (!gameState.editor.selectedEntity) return;

        const auto& translation = gameState.editor.selectedEntity->get<sill::TransformComponent>().translation();
        gameState.editor.gizmoEntity->get<sill::TransformComponent>().translation(translation);

        // @fixme Adapt size given distance to camera!
        gameState.editor.gizmoEntity->get<sill::TransformComponent>().scaling(glm::vec3(1.f));

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
    });
}
