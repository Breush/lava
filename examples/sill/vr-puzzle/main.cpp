#include <lava/sill.hpp>

#include "./camera.hpp"
#include "./environment.hpp"
#include "./game-logic.hpp"

using namespace lava;

int main(void)
{
    /**
     * @todo
     * - Some puzzle rules, and auto-check them.
     * - Visual feedback (success, failure), grabbing brick.
     */

    GameState gameState;

    sill::GameEngine engine;
    gameState.engine = &engine;

    // Camera (for companion window)
    setupCamera(gameState);

    // VR control
    engine.input().bindAction("trigger", VrButton::Trigger, VrDeviceType::RightHand);

    // Environment
    setupEnvironment(gameState);

    // Game logic
    setupGameLogic(gameState);

    engine.run();

    return EXIT_SUCCESS;
}
