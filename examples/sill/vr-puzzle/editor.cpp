#include "./editor.hpp"

#include <iostream>

using namespace lava;

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

        if (engine.input().justDown("left-fire")) {
            auto* entity = engine.pickEntity(gameState.pickingRay);
            if (entity) {
                std::cout << entity->name() << std::endl;
            }
        }
    });
}
