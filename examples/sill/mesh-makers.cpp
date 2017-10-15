/**
 * Shows the different integrated meshes.
 */

#include <lava/sill.hpp>

using namespace lava;

int main(void)
{
    sill::GameEngine engine;

    auto& corsetEntity = engine.make<sill::GameEntity>();
    auto& corsetMeshComponent = corsetEntity.make<sill::MeshComponent>();
    sill::makers::glbMeshMaker("./assets/models/corset.glb")(corsetMeshComponent);

    auto& planeEntity = engine.make<sill::GameEntity>();
    auto& planeMeshComponent = planeEntity.make<sill::MeshComponent>();
    sill::makers::planeMeshMaker({1, 1})(planeMeshComponent);
    planeEntity.get<sill::TransformComponent>().positionAdd({-1.2f, 0.f, 0.f});

    auto& sphereEntity = engine.make<sill::GameEntity>();
    auto& sphereMeshComponent = sphereEntity.make<sill::MeshComponent>();
    sill::makers::sphereMeshMaker(32u, 1.f)(sphereMeshComponent);
    sphereEntity.get<sill::TransformComponent>().positionAdd({1.2f, 0.f, 0.f});

    engine.run();

    return EXIT_SUCCESS;
}
