#include "./camera.hpp"
#include "./editor.hpp"
#include "./environment.hpp"
#include "./game-logic.hpp"
#include "./player.hpp"
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

    engine.registerMaterialFromFile("skybox", "./examples/sill/vr-puzzle/materials/skybox.shmag");
    engine.registerMaterialFromFile("barrier", "./examples/sill/vr-puzzle/materials/barrier.shmag");
    engine.registerMaterialFromFile("panel", "./examples/sill/vr-puzzle/materials/panel.shmag");
    engine.registerMaterialFromFile("collider", "./examples/sill/vr-puzzle/materials/collider.shmag");
    engine.registerMaterialFromFile("gizmo", "./examples/sill/vr-puzzle/materials/gizmo.shmag");
    engine.registerMaterialFromFile("reticle", "./examples/sill/vr-puzzle/materials/reticle.shmag");
    engine.registerMaterialFromFile("teleport-beam", "./examples/sill/vr-puzzle/materials/teleport-beam.shmag");
    engine.registerMaterialFromFile("teleport-area", "./examples/sill/vr-puzzle/materials/teleport-area.shmag");
    engine.registerMaterialFromFile("selection-rectangle", "./examples/sill/vr-puzzle/materials/selection-rectangle.shmag");
    engine.registerMaterialFromFile("water", "./examples/sill/vr-puzzle/materials/water.shmag");

    // @note Overiding default Roughness-Metallic material that automatically created in GLB loader.
    engine.registerMaterialFromFile("roughness-metallic", "./examples/sill/vr-puzzle/materials/roughness-metallic.shmag");

    // Camera (for companion window)
    setupCamera(gameState);

    // Player
    setupPlayer(gameState);

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
