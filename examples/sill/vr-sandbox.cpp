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

    // Spawning random cubes
    std::vector<sill::GameEntity*> cubes;
    for (auto i = 0u; i < 11u; ++i) {
        auto& cubeEntity = engine.make<sill::GameEntity>();
        auto& cubeMeshComponent = cubeEntity.make<sill::MeshComponent>();
        sill::makers::cubeMeshMaker(0.2f)(cubeMeshComponent);
        cubeEntity.get<sill::TransformComponent>().translate(
            {((rand() % 5) - 2.f) / 10.f, ((rand() % 5) - 2.f) / 10.f, 0.3f + (rand() % 20) / 10.f});
        cubeEntity.make<sill::BoxColliderComponent>();
        cubes.emplace_back(&cubeEntity);
    }

    // We bind our VR actions
    engine.input().bindAction("trigger", VrButton::Trigger, VrDeviceType::RightHand);

    // Behavior entity
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
        behaviorComponent.onUpdate([&]() {
            static sill::GameEntity* grabbedCube = nullptr;
            if (!engine.vrDeviceValid(VrDeviceType::RightHand)) return;

            auto handTransform = engine.vrDeviceTransform(VrDeviceType::RightHand);

            // When the user uses the trigger, we find the closest cube nearby, and grab it.
            if (engine.input().justDown("trigger")) {
                float minDistance = 1000.f;
                for (auto cube : cubes) {
                    auto cubeTranslation = cube->get<sill::TransformComponent>().translation();
                    auto distance = glm::distance(cubeTranslation, glm::vec3(handTransform[3]));
                    if (distance < minDistance) {
                        minDistance = distance;
                        grabbedCube = cube;
                    }
                }
            }
            else if (engine.input().justUp("trigger")) {
                grabbedCube = nullptr;
            }

            // Update cube to us whenever it is in grabbing state.
            if (grabbedCube != nullptr) {
                grabbedCube->get<sill::TransformComponent>().worldTransform(handTransform);

                // @fixme Would love to be able to get velocity of the hand,
                // and apply it to the cube!
            }
        });
    }

    app.engine().run();

    return EXIT_SUCCESS;
}
