/**
 * Generates a bouncy sphere on right click.
 */

#include "./ashe.hpp"

using namespace lava;

int main(void)
{
    ashe::Application app;
    auto& engine = app.engine();

    // Ground
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::planeMeshMaker({10, 10})(meshComponent);
        entity.make<sill::PlaneColliderComponent>();

        // @fixme How to say that this is static?
        // We might want a PhysicsComponent holding general data
    }

    // Generator entity
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
        behaviorComponent.onUpdate([&engine]() {
            // Generate a sphere on each right click
            if (engine.input().justDown("right-fire")) {
                // @todo Have a way to instantiate this entity - so that it shares meshes and collision data
                auto& entity = engine.make<sill::GameEntity>();
                auto& meshComponent = entity.make<sill::MeshComponent>();
                sill::makers::sphereMeshMaker(32u, 0.2f)(meshComponent);
                auto xOffset = (rand() % 100) / 500.f;
                auto yOffset = (rand() % 100) / 500.f;
                entity.get<sill::TransformComponent>().translate({xOffset, yOffset, 3});
                entity.make<sill::SphereColliderComponent>();
            }
        });

        engine.run();

        return EXIT_SUCCESS;
    }
}
