/**
 * Generates a bouncy sphere on right click.
 */

#include <lava/sill.hpp>

using namespace lava;

int main(void)
{
    sill::GameEngine engine;
    uint8_t frameCounter = 0;

    // Ground
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::planeMeshMaker({10, 10})(meshComponent);
        entity.make<sill::PlaneColliderComponent>();

        // @fixme How to say that this is static?
        // We might want a PhysicsComponent holding general data
    }

    // Generate a sphere on each right click
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
        behaviorComponent.onUpdate([&engine, &frameCounter]() {
            // @todo When input manager is here, remove this frame counter,
            // and generate a sphere on right click
            if (frameCounter++ == 0u) {
                auto& entity = engine.make<sill::GameEntity>();
                auto& meshComponent = entity.make<sill::MeshComponent>();
                sill::makers::sphereMeshMaker(32u, 0.2f)(meshComponent);
                auto offset = (rand() % 100) / 500.f;
                entity.get<sill::TransformComponent>().positionAdd({offset, offset, 3.f});
                entity.make<sill::SphereColliderComponent>();
            }
        });

        engine.run();

        return EXIT_SUCCESS;
    }
}
