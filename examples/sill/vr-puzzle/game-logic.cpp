#include "./game-logic.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include <lava/chamber/math.hpp>
#include <iostream>

#include "./environment.hpp"
#include "./ray-picking.hpp"

using namespace lava;

namespace {
    static Brick* grabbedBrick = nullptr;

    /// Checks whether the current level has been solved or not.
    bool checkLevelSolveStatus(GameState& gameState)
    {
        bool allPanelsSolved = true;
        for (auto& panel : gameState.level.panels) {
            allPanelsSolved = allPanelsSolved && panel->solved();
            if (!allPanelsSolved) {
                break;
            }
        }

        return allPanelsSolved;
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

    void grabBrick(GameState& gameState, Brick* brick)
    {
        gameState.state = State::GrabbedBrick;

        grabbedBrick = brick;
        grabbedBrick->unsnap();
        rayPickingEnabled(gameState, false);

        // Update the panel filling information.
        checkLevelSolveStatus(gameState);

        // Animate the world transform.
        grabbedBrick->animation().start(sill::AnimationFlag::WorldTransform, 0.2f, false);
    }

    void ungrabBrick(GameState& gameState)
    {
        if (!gameState.snapping.looksSnapped) {
            auto transform = glm::translate(glm::mat4(1.f), gameState.player.headPosition + gameState.player.direction);
            grabbedBrick->transform().worldTransform(transform);
        }

        gameState.state = State::Idle;
        grabbedBrick->animation().stop(sill::AnimationFlag::WorldTransform);
        grabbedBrick->selectionHighlighted(false);
        grabbedBrick->errorHighlighted(false);

        if (gameState.snapping.panel != nullptr) {
            grabbedBrick->snap(*gameState.snapping.panel, gameState.snapping.coordinates);
            grabbedBrick->baseRotationLevel(grabbedBrick->rotationLevel());
            grabbedBrick->extraRotationLevel(0u);
        }

        // Checking if the level is solved.
        if (checkLevelSolveStatus(gameState)) {
            levelSolved(gameState);
        }

        grabbedBrick = nullptr;
        rayPickingEnabled(gameState, true);
    }

    void rotateGrabbedBrick()
    {
        grabbedBrick->incrementBaseRotationLevel();
        grabbedBrick->animation().start(sill::AnimationFlag::WorldTransform, 0.1f, false);
    }

    glm::mat4 baseRotationLevelMatrix()
    {
        return glm::rotate(glm::mat4(1.f), grabbedBrick->baseRotationLevel() * 3.14156f * 0.5f, {0, 0, 1});
    }

    // Gameplay with VR controllers,
    // should not be mixed with mouse gameplay.
    void onUpdateVr(GameState& gameState)
    {
        auto& engine = *gameState.engine;
        if (!engine.vr().deviceValid(VrDeviceType::RightHand)) return;

        auto handTransform = engine.vr().deviceTransform(VrDeviceType::RightHand);

        // When the user uses the trigger, we find the closest brick nearby, and grab it.
        if (gameState.state == State::Idle && engine.input().justDown("trigger") && gameState.pointedBrick) {
            grabBrick(gameState, gameState.pointedBrick);
        }

        if (gameState.state != State::GrabbedBrick) return;

        if (engine.input().justUp("trigger")) {
            ungrabBrick(gameState);
            return;
        }

        // Update entity to us whenever it is in grabbing state.
        if (engine.input().justDown("touchpad")) {
            rotateGrabbedBrick();
        }

        // The user might be trying to turn is wrist instead of pushing the turning button.
        auto handRotationLevel = computeHandRotationLevel(handTransform);
        grabbedBrick->extraRotationLevel(handRotationLevel);

        // Offsetting from hand transform a little bit.
        auto targetTransform = glm::translate(handTransform, {0, 0, -0.2});
        targetTransform = glm::rotate(targetTransform, -3.14156f * 0.25f, {1, 0, 0});
        targetTransform = targetTransform * baseRotationLevelMatrix();

        // If the hand is close to a snapping point, we snap to it.
        gameState.snapping.panel = nullptr;
        for (auto& panel : gameState.level.panels) {
            if (auto snappingPoint = panel->closestSnappingPoint(*grabbedBrick, targetTransform[3])) {
                targetTransform = snappingPoint->worldTransform;
                targetTransform *= glm::rotate(glm::mat4(1.f), grabbedBrick->rotationLevel() * 3.14156f * 0.5f, {0, 0, 1});

                // Set the coordinates of snapped snapping point.
                gameState.snapping.panel = panel.get();
                gameState.snapping.coordinates = snappingPoint->coordinates;
                break;
            }
        }

        grabbedBrick->animation().target(sill::AnimationFlag::WorldTransform, targetTransform);
    }

    void onUpdateMouse(GameState& gameState)
    {
        auto& engine = *gameState.engine;

        // For mouse, the user uses click to grab then click to drop.
        if (engine.input().justDownUp("player.grab-brick")) {
            if (gameState.state == State::Idle && gameState.pointedBrick) {
                grabBrick(gameState, gameState.pointedBrick);
            }
            else if (gameState.state == State::GrabbedBrick) {
                ungrabBrick(gameState);
            }
        }

        if (gameState.state != State::GrabbedBrick) return;

        // Update entity to us whenever it is in grabbing state.
        if (engine.input().justDownUp("player.rotate-brick")) {
            rotateGrabbedBrick();
        }

        auto targetTransform = glm::mat4(1.f);

        // If the cursor is over a snapping point, we snap to it.
        gameState.snapping.looksSnapped = false;
        gameState.snapping.panel = nullptr;
        for (auto& panel : gameState.level.panels) {
            if (!panel->userInteractionAllowed()) continue;

            // @todo We should find out which panel is the closest!
            Panel::SnappingInfo snappingInfo = panel->rayHitSnappingPoint(*grabbedBrick, gameState.pickingRay);
            if (snappingInfo.point != nullptr) {
                gameState.snapping.looksSnapped = true;
                targetTransform = snappingInfo.point->worldTransform;
                targetTransform *= baseRotationLevelMatrix();

                // Set the coordinates of snapped snapping point.
                if (snappingInfo.validForBrick) {
                    grabbedBrick->errorHighlighted(false);
                    gameState.snapping.panel = panel.get();
                    gameState.snapping.coordinates = snappingInfo.point->coordinates;
                }
                else {
                    grabbedBrick->errorHighlighted(true);
                }

                break;
            }
        }

        // If the brick is not snapped, we move the brick to the lower right corner of the screen.
        if (!gameState.snapping.looksSnapped) {
            grabbedBrick->errorHighlighted(false);

            const auto& extent = gameState.camera.component->extent();
            auto coordinates = glm::vec2{0.9f * extent.width, 0.9f * extent.height};
            auto screenMatrix = gameState.camera.component->unprojectAsTransform(coordinates, 0.5f);
            screenMatrix = glm::rotate(screenMatrix, chamber::math::PI_OVER_TWO, {0, 1, 0});
            screenMatrix = glm::scale(screenMatrix, glm::vec3{0.02f});

            targetTransform = screenMatrix * baseRotationLevelMatrix();
        }

        grabbedBrick->animation().target(sill::AnimationFlag::WorldTransform, targetTransform);
    }
}

void setupGameLogic(GameState& gameState)
{
    auto& entity = gameState.engine->make<sill::GameEntity>("game-logic");
    auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
    behaviorComponent.onUpdate([&](float /* dt */) {
        if (gameState.engine->vr().enabled()) {
            onUpdateVr(gameState);
        }
        else {
            onUpdateMouse(gameState);
        }

        // Teleport back brick that are infinitely falling.
        if (gameState.state != State::Editor) {
            for (auto& brick : gameState.level.bricks) {
                if (brick->transform().translation().z < -20.f) {
                    auto translation = brick->transform().translation();
                    translation.z += 100.f;

                    Ray ray;
                    ray.origin = translation;
                    ray.direction = glm::vec3{0.f, 0.f, -1.f};
                    auto distance = gameState.terrain.entity->distanceFrom(ray);
                    if (distance != 0.f) {
                        translation = ray.origin + (distance - 0.1f) * ray.direction;
                    }

                    brick->transform().translation(translation);
                }
            }
        }
    });
}
