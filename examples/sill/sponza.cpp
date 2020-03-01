/**
 * The classic sponza scene.
 */

#include "./ashe.hpp"

using namespace lava;

#include <iostream>

int main(void)
{
    ashe::Application app;
    auto& engine = app.engine();

    auto& entity = engine.make<sill::GameEntity>();
    auto& meshComponent = entity.make<sill::MeshComponent>();
    sill::makers::glbMeshMaker("./assets/models/sponza.glb")(meshComponent);

    engine.run();

    return EXIT_SUCCESS;
}
