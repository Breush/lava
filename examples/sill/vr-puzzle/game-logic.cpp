#include "./game-logic.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace lava;

void setupGameLogic(GameState& gameState)
{
    auto& engine = *gameState.engine;
    auto& entity = engine.make<sill::GameEntity>();
    auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
    behaviorComponent.onUpdate([&]() {
        static sill::GameEntity* grabbedEntity = nullptr;
        static bool grabbedEntityIsSnapped = false;

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

            // If the user let go the brick while it is snapped to a binding point,
            // we disable physics.
            grabbedEntity->get<sill::BoxColliderComponent>().enabled(!grabbedEntityIsSnapped);

            grabbedEntity = nullptr;
        }

        // Update entity to us whenever it is in grabbing state.
        if (grabbedEntity != nullptr) {
            // Offsetting from hand transform a little bit.
            auto targetTransform = glm::translate(handTransform, {0, 0, -0.2});
            targetTransform = glm::rotate(targetTransform, -3.14156f * 0.25f, {1, 0, 0});

            // If the hand is close to a binding point, we snap to it.
            float minDistance = 0.1f;
            grabbedEntityIsSnapped = false;
            for (auto& tableBindingNodes : gameState.tableBindingNodes) {
                for (auto node : tableBindingNodes) {
                    auto distance = glm::distance(glm::vec3(node->worldTransform[3]), glm::vec3(targetTransform[3]));
                    if (distance < minDistance) {
                        minDistance = distance;
                        targetTransform = node->worldTransform;
                        targetTransform = glm::rotate(targetTransform, -3.14156f * 0.125f, {1, 0, 0});
                        grabbedEntityIsSnapped = true;
                    }
                }
            }

            grabbedEntity->get<sill::AnimationComponent>().target(sill::AnimationFlag::WorldTransform, targetTransform);
        }
    });
}
