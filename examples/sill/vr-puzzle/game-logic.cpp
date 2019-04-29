#include "./game-logic.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <iostream>

#include "./environment.hpp"

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

    /**
     * Checks whether the binding point is valid for the brick.
     *
     * Valid: no overlapping with other bricks, completely within the panel dimensions.
     */
    bool isBindingPointValid(GameState& gameState, const Brick& brick, const BindingPoint& bindingPoint)
    {
        // @fixme This could be moved to Panel, like tableBindingPoints.filled

        for (const auto& block : brick.blocks) {
            auto i = bindingPoint.coordinates.x + block.x;
            auto j = bindingPoint.coordinates.y + block.y;

            // Check that the block coordinates are valid.
            if (i >= gameState.tableBindingPoints.size()) return false;
            if (j >= gameState.tableBindingPoints[i].size()) return false;

            // Check that the block coordinates are empty.
            if (gameState.tableBindingPoints[i][j].filled) return false;
        }

        return true;
    }

    /// Should be called each time rotationLevel is changed.
    void updateBrickBlocks(Brick& brick)
    {
        brick.blocks = brick.nonRotatedBlocks;

        for (auto& block : brick.blocks) {
            // X <= -Y and Y <=  X is a 90° clockwise rotation
            for (auto k = 0u; k < brick.rotationLevel; ++k) {
                std::swap(block.x, block.y);
                block.x = -block.x;
            }
        }
    }

    /// Find the closest MeshNode for the table binding points.
    BindingPoint* findClosestTableBindingPoint(GameState& gameState, const Brick& brick, const glm::vec3& position,
                                               float minDistance = 0.1f)
    {
        BindingPoint* closestBindingPoint = nullptr;

        for (auto& tableBindingPoints : gameState.tableBindingPoints) {
            for (auto& bindingPoint : tableBindingPoints) {
                auto distance = glm::distance(glm::vec3(bindingPoint.node->worldTransform[3]), position);
                if (distance < minDistance) {
                    if (!isBindingPointValid(gameState, brick, bindingPoint)) continue;
                    minDistance = distance;
                    closestBindingPoint = &bindingPoint;
                }
            }
        }

        return closestBindingPoint;
    }

    /// Checks whether the current panel has been solved or not.
    bool checkPanelSolveStatus(GameState& gameState)
    {
        auto& tableAnimation = gameState.tableEntity->get<sill::AnimationComponent>();

        // Check that the panel is all right.
        if (!gameState.panel.checkSolveStatus(gameState)) {
            // Visual feedback: unsolved panels are white.
            tableAnimation.start(sill::AnimationFlag::MaterialUniform, *gameState.tableMaterial, "albedoColor", 0.1f);
            tableAnimation.target(sill::AnimationFlag::MaterialUniform, *gameState.tableMaterial, "albedoColor",
                                  glm::vec4{1.f, 1.f, 1.f, 1.f});
            return false;
        }

        // Visual feedback: solved panels turn green.
        tableAnimation.start(sill::AnimationFlag::MaterialUniform, *gameState.tableMaterial, "albedoColor", 0.5f);
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
        if (engine.input().justDown("trigger")) {
            grabbedBrick = findClosestBrick(gameState, handTransform[3]);
            grabbedBrick->snapped = false;

            // Update the panel filling information.
            checkPanelSolveStatus(gameState);

            // We will animate the world transform over 300ms.
            grabbedBrick->entity->get<sill::AnimationComponent>().start(sill::AnimationFlag::WorldTransform, 0.2f);
        }
        else if (engine.input().justUp("trigger")) {
            grabbedBrick->entity->get<sill::AnimationComponent>().stop(sill::AnimationFlag::WorldTransform);
            grabbedBrick->buttonRotationLevel = grabbedBrick->rotationLevel;

            // If the user let go the brick while it is snapped to a binding point,
            // we disable physics.
            grabbedBrick->entity->get<sill::BoxColliderComponent>().enabled(!grabbedBrick->snapped);

            // Checking if the panel is solved.
            if (checkPanelSolveStatus(gameState)) {
                loadLevel(gameState, gameState.levelId + 1);
            }

            grabbedBrick = nullptr;
        }

        // Update entity to us whenever it is in grabbing state.
        if (grabbedBrick != nullptr) {
            // Rotate the entity when touchpad is pressed
            if (engine.input().justDown("touchpad")) {
                grabbedBrick->buttonRotationLevel = (grabbedBrick->buttonRotationLevel + 1) % 4;
                grabbedBrick->entity->get<sill::AnimationComponent>().start(sill::AnimationFlag::WorldTransform, 0.1f);
            }

            // The user might be trying to turn is wrist instead of pushing the turning button.
            auto handRotationLevel = computeHandRotationLevel(handTransform);
            auto rotationLevel = (grabbedBrick->buttonRotationLevel + handRotationLevel) % 4;
            if (rotationLevel != grabbedBrick->rotationLevel) {
                grabbedBrick->rotationLevel = rotationLevel;
                updateBrickBlocks(*grabbedBrick);
            }

            // Offsetting from hand transform a little bit.
            auto targetTransform = glm::translate(handTransform, {0, 0, -0.2});
            targetTransform = glm::rotate(targetTransform, -3.14156f * 0.25f, {1, 0, 0});
            targetTransform = glm::rotate(targetTransform, grabbedBrick->buttonRotationLevel * 3.14156f * 0.5f, {0, 0, 1});

            // If the hand is close to a binding point, we snap to it.
            grabbedBrick->snapped = false;
            if (auto bindingPoint = findClosestTableBindingPoint(gameState, *grabbedBrick, targetTransform[3])) {
                targetTransform = bindingPoint->node->worldTransform;
                targetTransform = glm::rotate(targetTransform, -3.14156f * 0.125f, {1, 0, 0});
                targetTransform = glm::rotate(targetTransform, grabbedBrick->rotationLevel * 3.14156f * 0.5f, {0, 0, 1});

                // Set the coordinates of snapped binding point.
                grabbedBrick->snapped = true;
                grabbedBrick->snapCoordinates = bindingPoint->coordinates;
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
