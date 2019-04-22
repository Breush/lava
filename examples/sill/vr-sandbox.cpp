/**
 * Shows the different integrated meshes.
 */

#include "./ashe.hpp"

#include <iostream>

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
    }

    // Cube mesh
    auto& cubeEntity = engine.make<sill::GameEntity>();
    auto& cubeMeshComponent = cubeEntity.make<sill::MeshComponent>();
    sill::makers::cubeMeshMaker(0.2f)(cubeMeshComponent);
    cubeEntity.get<sill::TransformComponent>().translate({0.f, -1.2f, 2.25f});
    cubeEntity.make<sill::BoxColliderComponent>();

    // We bind our VR actions
    engine.input().bindAction("trigger", VrButton::Trigger, VrDeviceType::RightHand);

    // Behavior entity
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
        behaviorComponent.onUpdate([&]() {
            static bool grabbingCube = false;

            if (engine.input().justDown("trigger")) {
                /**
                 * We want to see if we are over the cube mesh,
                 * and if so, start "grabing" it. Making it follow the moving controller.
                 * @fixme For now, we just say everything is all right.
                 */
                grabbingCube = true;

                // @fixme We should have a way to stop physics for a while.
                // This is working because we are forcing the transform, but still...
                // it depends on component order on this example.
            }
            else if (engine.input().justUp("trigger")) {
                grabbingCube = false;
            }

            // Update cube to us whenever it is in grabbing state.
            if (grabbingCube) {
                auto handTransform = engine.vrDeviceTransform(VrDeviceType::RightHand);
                cubeEntity.get<sill::TransformComponent>().worldTransform(handTransform);

                // @fixme Would love to be able to get velocity of the hand,
                // and apply it to the cube!
            }
        });
    }

    app.engine().run();

    return EXIT_SUCCESS;
}
