/**
 * Shows the different integrated meshes.
 */

#include "./ashe.hpp"

using namespace lava;

int main(void)
{
    ashe::Application app;
    auto& engine = app.engine();

    // Ground
    {
        auto& entity = engine.make<sill::Entity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::planeMeshMaker({10, 10})(meshComponent);
        entity.make<sill::ColliderComponent>();
        entity.get<sill::ColliderComponent>().addInfinitePlaneShape();
        entity.get<sill::PhysicsComponent>().dynamic(false);
    }

    // Spawning random cubes
    std::vector<sill::Entity*> cubes;
    for (auto i = 0u; i < 10u; ++i) {
        auto cubeSize = (2 + rand() % 20) / 40.f;
        auto& cubeEntity = engine.make<sill::Entity>();
        auto& cubeMeshComponent = cubeEntity.make<sill::MeshComponent>();
        sill::makers::boxMeshMaker(cubeSize)(cubeMeshComponent);
        cubeEntity.get<sill::TransformComponent>().translate({(rand() % 5) / 10.f - 0.2f, // X
                                                              (rand() % 5) / 10.f - 0.2f, // Y
                                                              0.3f + (rand() % 20) / 10.f});
        cubeEntity.make<sill::ColliderComponent>();
        cubeEntity.get<sill::ColliderComponent>().addBoxShape({0.f, 0.f, 0.f}, cubeSize);
        cubeEntity.make<sill::AnimationComponent>();
        cubes.emplace_back(&cubeEntity);
    }

    // We bind our VR actions
    engine.input().bindAction("trigger", VrButton::Trigger, VrDeviceType::RightHand);

    // Behavior entity
    {
        auto& entity = engine.make<sill::Entity>();
        auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
        behaviorComponent.onUpdate([&](float /* dt */) {
            static sill::Entity* grabbedCube = nullptr;
            if (!engine.vr().deviceValid(VrDeviceType::RightHand)) return;

            const auto& handTransform = engine.vr().deviceTransform(VrDeviceType::RightHand);

            // When the user uses the trigger, we find the closest cube nearby, and grab it.
            if (engine.input().justDown("trigger")) {
                float minDistance = 1000.f;
                for (auto cube : cubes) {
                    auto cubeTranslation = cube->get<sill::TransformComponent>().translation();
                    auto distance = glm::distance(cubeTranslation, handTransform.translation);
                    if (distance < minDistance) {
                        minDistance = distance;
                        grabbedCube = cube;
                    }
                }

                // We will animate the world transform over 300ms.
                grabbedCube->get<sill::AnimationComponent>().start(sill::AnimationFlag::WorldTransform, 0.3f, false);
            }
            else if (engine.input().justUp("trigger")) {
                grabbedCube->get<sill::AnimationComponent>().stop(sill::AnimationFlag::WorldTransform);
                grabbedCube = nullptr;
            }

            // Update cube to us whenever it is in grabbing state.
            if (grabbedCube != nullptr) {
                grabbedCube->get<sill::AnimationComponent>().target(sill::AnimationFlag::WorldTransform, handTransform);

                // @fixme Would love to be able to get velocity of the hand,
                // and apply it to the cube!
            }
        });
    }

    app.engine().run();

    return EXIT_SUCCESS;
}
