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
        entity.make<sill::ColliderComponent>();
        entity.get<sill::ColliderComponent>().addInfinitePlaneShape();
        entity.get<sill::PhysicsComponent>().dynamic(false);
    }

    // Generator entity
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
        behaviorComponent.onUpdate([&engine]() {
            // Generate a sphere on each right click
            if (engine.input().justDown("right-fire")) {
                auto sphereDiameter = (2 + rand() % 20) / 40.f;
                auto& entity = engine.make<sill::GameEntity>();
                auto& meshComponent = entity.make<sill::MeshComponent>();
                sill::makers::sphereMeshMaker(32u, sphereDiameter)(meshComponent);
                auto xOffset = (rand() % 100) / 500.f;
                auto yOffset = (rand() % 100) / 500.f;
                entity.make<sill::ColliderComponent>();
                entity.get<sill::ColliderComponent>().addSphereShape({0.f, 0.f, 0.f}, sphereDiameter);
                entity.get<sill::TransformComponent>().translate({xOffset, yOffset, 3});
            }
        });

        engine.run();

        return EXIT_SUCCESS;
    }
}
