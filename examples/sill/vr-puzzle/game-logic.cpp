#include "./game-logic.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace lava;

namespace {
    /// Find the closest brick according the specified position.
    Brick* findClosestBrick(GameState& gameState, const glm::vec3& position, float minDistance = 1000.f)
    {
        Brick* closestBrick = nullptr;

        for (auto& brick : gameState.bricks) {
            auto brickTranslation = brick.entity->get<sill::TransformComponent>().translation();
            auto distance = glm::distance(brickTranslation, position);
            if (distance < minDistance) {
                minDistance = distance;
                closestBrick = &brick;
            }
        }

        return closestBrick;
    }

    /// Find the closest MeshNode for the table binding points.
    const sill::MeshNode* findClosestTableBindingNode(GameState& gameState, const glm::vec3& position, float minDistance = 0.1f)
    {
        const sill::MeshNode* closestNode = nullptr;

        for (auto& tableBindingNodes : gameState.tableBindingNodes) {
            for (auto node : tableBindingNodes) {
                auto distance = glm::distance(glm::vec3(node->worldTransform[3]), position);
                if (distance < minDistance) {
                    minDistance = distance;
                    closestNode = node;
                }
            }
        }

        return closestNode;
    }

    void onUpdate(GameState& gameState)
    {
        static Brick* grabbedBrick = nullptr;

        auto& engine = *gameState.engine;
        if (!engine.vrDeviceValid(VrDeviceType::RightHand)) return;

        auto handTransform = engine.vrDeviceTransform(VrDeviceType::RightHand);

        // When the user uses the trigger, we find the closest brick nearby, and grab it.
        if (engine.input().justDown("trigger")) {
            grabbedBrick = findClosestBrick(gameState, handTransform[3]);

            // We will animate the world transform over 300ms.
            grabbedBrick->entity->get<sill::AnimationComponent>().start(sill::AnimationFlag::WorldTransform, 0.2f);
        }
        else if (engine.input().justUp("trigger")) {
            grabbedBrick->entity->get<sill::AnimationComponent>().stop(sill::AnimationFlag::WorldTransform);

            // If the user let go the brick while it is snapped to a binding point,
            // we disable physics.
            grabbedBrick->entity->get<sill::BoxColliderComponent>().enabled(!grabbedBrick->snapped);

            grabbedBrick = nullptr;
        }

        // Update entity to us whenever it is in grabbing state.
        if (grabbedBrick != nullptr) {
            // Rotate the entity when touchpad is pressed
            if (engine.input().justDown("touchpad")) {
                grabbedBrick->rotationLevel = (grabbedBrick->rotationLevel + 1) % 4;
                grabbedBrick->entity->get<sill::AnimationComponent>().start(sill::AnimationFlag::WorldTransform, 0.1f);
            }

            // Offsetting from hand transform a little bit.
            auto targetTransform = glm::translate(handTransform, {0, 0, -0.2});
            targetTransform = glm::rotate(targetTransform, -3.14156f * 0.25f, {1, 0, 0});
            targetTransform = glm::rotate(targetTransform, grabbedBrick->rotationLevel * 3.14156f * 0.5f, {0, 0, 1});

            // If the hand is close to a binding point, we snap to it.
            grabbedBrick->snapped = false;
            if (auto node = findClosestTableBindingNode(gameState, targetTransform[3])) {
                targetTransform = node->worldTransform;
                targetTransform = glm::rotate(targetTransform, -3.14156f * 0.125f, {1, 0, 0});
                targetTransform = glm::rotate(targetTransform, grabbedBrick->rotationLevel * 3.14156f * 0.5f, {0, 0, 1});
                grabbedBrick->snapped = true;
            }

            grabbedBrick->entity->get<sill::AnimationComponent>().target(sill::AnimationFlag::WorldTransform, targetTransform);
        }
    }
}

void setupGameLogic(GameState& gameState)
{
    auto& entity = gameState.engine->make<sill::GameEntity>();
    auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
    behaviorComponent.onUpdate([&]() { onUpdate(gameState); });
}
