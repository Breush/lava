#include "./game-logic.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <iostream>

#include "./environment.hpp"

using namespace lava;

namespace {
    /// Checks whether the current panel has been solved or not.
    bool checkPanelSolveStatus(GameState& gameState)
    {
        auto& tableAnimation = gameState.tableEntity->get<sill::AnimationComponent>();

        // Check that the panel is all right.
        if (!gameState.panel.checkSolveStatus(gameState)) {
            // Visual feedback: unsolved panels are white.
            tableAnimation.start(sill::AnimationFlag::MaterialUniform, *gameState.tableMaterial, "albedoColor", 0.5f);
            tableAnimation.target(sill::AnimationFlag::MaterialUniform, *gameState.tableMaterial, "albedoColor",
                                  glm::vec4{1.f, 1.f, 1.f, 1.f});
            return false;
        }

        // Visual feedback: solved panels turn green.
        tableAnimation.start(sill::AnimationFlag::MaterialUniform, *gameState.tableMaterial, "albedoColor", 0.1f);
        tableAnimation.target(sill::AnimationFlag::MaterialUniform, *gameState.tableMaterial, "albedoColor",
                              glm::vec4{0.46, 0.86, 0.46, 1.f});

        // All bricks where fine, panel is filled.
        return true;
    }

    /// Extract the rotation level of a transform.
    uint32_t computeHandRotationLevel(const glm::mat4& transform)
    {
        glm::vec3 scale;
        glm::quat orientation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(transform, scale, orientation, translation, skew, perspective);

        auto eulerAngles = glm::eulerAngles(orientation);

        int32_t handRotationLevel = std::round(-eulerAngles.y / (3.14156f / 2.f));
        if (handRotationLevel == 0 && eulerAngles.x < 0.f) handRotationLevel += 2;

        return (4 + handRotationLevel) % 4;
    }

    void onUpdate(GameState& gameState)
    {
        static Brick* grabbedBrick = nullptr;

        auto& engine = *gameState.engine;
        if (!engine.vrDeviceValid(VrDeviceType::RightHand)) return;

        auto handTransform = engine.vrDeviceTransform(VrDeviceType::RightHand);

        // When the user uses the trigger, we find the closest brick nearby, and grab it.
        if (engine.input().justDown("trigger") && gameState.pointedBrick != nullptr) {
            grabbedBrick = gameState.pointedBrick;
            grabbedBrick->unsnap();
            rayPickingEnabled(gameState, false);

            // Update the panel filling information.
            checkPanelSolveStatus(gameState);

            // We will animate the world transform over 300ms.
            grabbedBrick->animation().start(sill::AnimationFlag::WorldTransform, 0.2f);
        }
        else if (engine.input().justUp("trigger") && grabbedBrick != nullptr) {
            grabbedBrick->animation().stop(sill::AnimationFlag::WorldTransform);
            grabbedBrick->baseRotationLevel(grabbedBrick->rotationLevel());
            grabbedBrick->extraRotationLevel(0u);

            // Checking if the panel is solved.
            if (checkPanelSolveStatus(gameState)) {
                // Dropping all bricks
                // @fixme Add a timer before dropping everything!
                for (auto& brick : gameState.bricks) {
                    brick.unsnap();
                }
                gameState.panel.checkSolveStatus(gameState);

                loadLevel(gameState, gameState.levelId + 1);

                // Setting table to unsolved status.
                auto& tableAnimation = gameState.tableEntity->get<sill::AnimationComponent>();
                tableAnimation.start(sill::AnimationFlag::MaterialUniform, *gameState.tableMaterial, "albedoColor", 0.1f);
                tableAnimation.target(sill::AnimationFlag::MaterialUniform, *gameState.tableMaterial, "albedoColor",
                                      glm::vec4{1.f, 1.f, 1.f, 1.f});
            }

            grabbedBrick = nullptr;
            rayPickingEnabled(gameState, true);
        }

        // Update entity to us whenever it is in grabbing state.
        if (grabbedBrick != nullptr) {
            // Rotate the entity when touchpad is pressed
            if (engine.input().justDown("touchpad")) {
                grabbedBrick->incrementBaseRotationLevel();
                grabbedBrick->animation().start(sill::AnimationFlag::WorldTransform, 0.1f);
            }

            // The user might be trying to turn is wrist instead of pushing the turning button.
            auto handRotationLevel = computeHandRotationLevel(handTransform);
            grabbedBrick->extraRotationLevel(handRotationLevel);

            // Offsetting from hand transform a little bit.
            auto targetTransform = glm::translate(handTransform, {0, 0, -0.2});
            targetTransform = glm::rotate(targetTransform, -3.14156f * 0.25f, {1, 0, 0});
            targetTransform = glm::rotate(targetTransform, grabbedBrick->baseRotationLevel() * 3.14156f * 0.5f, {0, 0, 1});

            // If the hand is close to a binding point, we snap to it.
            if (auto bindingPoint = gameState.panel.closestBindingPoint(*grabbedBrick, targetTransform[3])) {
                targetTransform = bindingPoint->worldTransform;
                targetTransform *= glm::rotate(glm::mat4(1.f), grabbedBrick->rotationLevel() * 3.14156f * 0.5f, {0, 0, 1});

                // Set the coordinates of snapped binding point.
                grabbedBrick->snap(bindingPoint->coordinates);
            }
            else {
                grabbedBrick->unsnap();
            }

            grabbedBrick->animation().target(sill::AnimationFlag::WorldTransform, targetTransform);
        }
    }
}

void setupGameLogic(GameState& gameState)
{
    auto& entity = gameState.engine->make<sill::GameEntity>();
    auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
    behaviorComponent.onUpdate([&]() { onUpdate(gameState); });
}
