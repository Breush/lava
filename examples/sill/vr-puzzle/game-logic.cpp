#include "./game-logic.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace lava;

void setupGameLogic(GameState& gameState)
{
    // @fixme We need some binding points for the bricks
    // to be snapped on the puzzle table.

    auto& engine = *gameState.engine;
    auto& entity = engine.make<sill::GameEntity>();
    auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
    behaviorComponent.onUpdate([&]() {
        static sill::GameEntity* grabbedEntity = nullptr;
        if (!engine.vrDeviceValid(VrDeviceType::RightHand)) return;

        auto handTransform = engine.vrDeviceTransform(VrDeviceType::RightHand);

        // When the user uses the trigger, we find the closest brick nearby, and grab it.
        if (engine.input().justDown("trigger")) {
            float minDistance = 1000.f;
            for (auto entity : gameState.draggableEntities) {
                auto entityTranslation = entity->get<sill::TransformComponent>().translation();
                auto distance = glm::distance(entityTranslation, glm::vec3(handTransform[3]));
                if (distance < minDistance) {
                    minDistance = distance;
                    grabbedEntity = entity;
                }
            }

            // We will animate the world transform over 300ms.
            grabbedEntity->get<sill::AnimationComponent>().start(sill::AnimationFlag::WorldTransform, 0.2f);
        }
        else if (engine.input().justUp("trigger")) {
            grabbedEntity->get<sill::AnimationComponent>().stop(sill::AnimationFlag::WorldTransform);
            grabbedEntity = nullptr;
        }

        // Update entity to us whenever it is in grabbing state.
        if (grabbedEntity != nullptr) {
            auto targetTransform = glm::translate(handTransform, {0, 0, -0.2});
            targetTransform = glm::rotate(targetTransform, -3.14156f * 1.25f, {1, 0, 0});
            grabbedEntity->get<sill::AnimationComponent>().target(sill::AnimationFlag::WorldTransform, targetTransform);
        }
    });
}
