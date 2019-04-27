#include <lava/sill.hpp>

#include "./camera.hpp"
#include "./environment.hpp"
#include "./game-logic.hpp"

using namespace lava;

int main(void)
{
    /**
     * @todo
     * - Some more puzzle rules.
     * - Visual feedback (success, failure), grabbed brick identification.
     * - Raycast to pick pointed brick.
     */

    GameState gameState;

    sill::GameEngine engine;
    gameState.engine = &engine;

    // Camera (for companion window)
    setupCamera(gameState);

    // VR control
    engine.input().bindAction("trigger", VrButton::Trigger, VrDeviceType::RightHand);
    engine.input().bindAction("touchpad", VrButton::Touchpad, VrDeviceType::RightHand);

    // Environment
    setupEnvironment(gameState);

    // Game logic
    setupGameLogic(gameState);

    // Load first level
    loadLevel(gameState, 0);

    engine.run();

    return EXIT_SUCCESS;
}
