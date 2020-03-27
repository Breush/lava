#include <lava/sill.hpp>

#include "./camera.hpp"
#include "./editor.hpp"
#include "./environment.hpp"
#include "./game-logic.hpp"
#include "./ray-picking.hpp"
#include "./teleport-beam.hpp"

using namespace lava;

int main(void)
{
    /**
     * @todo
     * - Some more puzzle rules.
     * - Visual feedback (panel failure, explain why), better grabbed brick identification.
     * - Mesh instances so that bricks are not duplicated. Block's material can be shared per brick.
     */

    sill::GameEngine engine;

    GameState gameState;
    gameState.engine = &engine;

    engine.registerMaterialFromFile("skybox", "./data/shaders/materials/skybox-material.shmag");
    engine.registerMaterialFromFile("barrier", "./examples/sill/vr-puzzle/materials/barrier.shmag");
    engine.registerMaterialFromFile("panel", "./examples/sill/vr-puzzle/materials/panel.shmag");
    engine.registerMaterialFromFile("gizmo", "./examples/sill/vr-puzzle/materials/gizmo.shmag");
    engine.registerMaterialFromFile("reticle", "./examples/sill/vr-puzzle/materials/reticle.shmag");
    engine.registerMaterialFromFile("teleport-beam", "./examples/sill/vr-puzzle/materials/teleport-beam.shmag");
    engine.registerMaterialFromFile("teleport-area", "./examples/sill/vr-puzzle/materials/teleport-area.shmag");

    // Camera (for companion window)
    setupCamera(gameState);

    // VR control
    engine.input().bindAction("trigger", VrButton::Trigger, VrDeviceType::RightHand);
    engine.input().bindAction("touchpad", VrButton::Touchpad, VrDeviceType::RightHand);

    // Environment
    setupEnvironment(gameState);
    setupRayPicking(gameState);
    setupTeleportBeam(gameState);

    // Game logic
    setupGameLogic(gameState);

    // Editor
    setupEditor(gameState);

    // Load first level
    loadLevel(gameState, "./examples/sill/vr-puzzle/level-intro.json");

    engine.run();

    return EXIT_SUCCESS;
}
