/**
 * The classic sponza scene.
 */

#include <lava/sill.hpp>

using namespace lava;

int main(void)
{
    sill::GameEngine engine;

    auto& entity = engine.make<sill::GameEntity>();
    auto& meshComponent = entity.make<sill::MeshComponent>();
    sill::makers::glbMeshMaker("./assets/models/sponza.glb")(meshComponent);

    engine.run();

    return EXIT_SUCCESS;
}
