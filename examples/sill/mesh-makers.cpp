/**
 * Shows the different integrated meshes.
 */

#include "./ashe.hpp"

using namespace lava;

int main(void)
{
    ashe::Application app;
    auto& engine = app.engine();

    // GLB mesh maker
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::glbMeshMaker("./assets/models/boom-box.glb")(meshComponent);
        entity.get<sill::TransformComponent>().translate({1.2f, 0.f, 0});
        entity.get<sill::TransformComponent>().rotate({0.f, 0.f, 1.f}, 3.14f);
        entity.get<sill::TransformComponent>().scale(40.f);
    }

    // Plane mesh maker
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::planeMeshMaker({1, 1})(meshComponent);
        entity.get<sill::TransformComponent>().translate({0.f, 1.2f, 0.f});
    }

    // Cube mesh maker
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::boxMeshMaker(1.f)(meshComponent);
        entity.get<sill::TransformComponent>().translate({0.f, -1.2f, 0.f});
    }

    // Sphere mesh maker
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::sphereMeshMaker(32u, 1.f)(meshComponent);
        entity.get<sill::TransformComponent>().translate({-1.2f, 0.f, 0.f});
    }

    app.engine().run();

    return EXIT_SUCCESS;
}
