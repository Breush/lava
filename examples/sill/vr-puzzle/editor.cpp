#include "./editor.hpp"

#include <iostream>

using namespace lava;

namespace {
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

    auto& editorEntity = engine.make<sill::GameEntity>("editor");
    auto& editorBehavior = editorEntity.make<sill::BehaviorComponent>();

    engine.input().bindAction("editorToggle", Key::E);

    editorBehavior.onUpdate([&](float /* dt */) {
        if ((gameState.state == State::Idle || gameState.state == State::Editor) && engine.input().justDown("editorToggle")) {
            gameState.state = (gameState.state == State::Editor) ? State::Idle : State::Editor;
            std::cout << "Editor: " << (gameState.state == State::Editor) << std::endl;
        }

        if (gameState.state != State::Editor) return;

        // Select entity on left click if any.
        if (engine.input().justDown("left-fire")) {
            auto* selectedEntity = engine.pickEntity(gameState.pickingRay);

            // Find the "root" entity of the selected mesh.
            if (selectedEntity) {
                while (selectedEntity->parent()) {
                    selectedEntity = selectedEntity->parent();
                }
            }

            if (gameState.editor.selectedEntity != selectedEntity) {
                setCategoryForHierarchy(gameState.editor.selectedEntity, RenderCategory::Opaque);
            }

            gameState.editor.selectedEntity = selectedEntity;
            setCategoryForHierarchy(selectedEntity, RenderCategory::Wireframe);

            if (!selectedEntity) return;
        }
    });
}
