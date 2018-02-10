/**
 * Generates a bouncy sphere on right click.
 */

#include <lava/sill.hpp>

using namespace lava;

int main(void)
{
    sill::GameEngine engine;

    // Ground
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::planeMeshMaker({10, 10})(meshComponent);
        entity.make<sill::PlaneColliderComponent>();

        // @fixme How to say that this is static?
        // We might want a PhysicsComponent holding general data
    }

    // @fixme Generate spheres on right click

    // A sphere
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::sphereMeshMaker(32u, 0.1f)(meshComponent);
        entity.get<sill::TransformComponent>().positionAdd({0.f, 0.f, 5.f});
        entity.make<sill::SphereColliderComponent>();
    }

    engine.run();

    return EXIT_SUCCESS;
}
