#include "./ray-picking.hpp"

#include <glm/gtx/string_cast.hpp>
#include <iostream>

using namespace lava;

namespace {
    bool g_rayPickingEnabled = true;

    Brick* pickBrick(GameState& gameState, const Ray& ray)
    {
        const auto bsRadius = std::sqrt(blockExtent.x * blockExtent.x + blockExtent.y * blockExtent.y) / 2.f;

        // @fixme Should be done through GameEngine API with raycast colliders?
        Brick* pickedBrick = nullptr;

        float minDistance = 50.f;
        for (auto& brick : gameState.bricks) {
            for (auto& block : brick->blocks()) {
                // @todo For now, this is just a arbitrary ray-sphere detection,
                // we could do ray-box intersection.
                glm::vec3 bsCenter = block.entity->get<sill::TransformComponent>().worldTransform()[3];
                auto rayOriginToBsCenter = bsCenter - ray.origin;
                auto bsCenterProjectionDistance = glm::dot(ray.direction, rayOriginToBsCenter);
                if (bsCenterProjectionDistance > 0.f) {
                    if (std::abs(bsCenterProjectionDistance) < minDistance) {
                        auto bsCenterProjection = ray.origin + ray.direction * bsCenterProjectionDistance;
                        if (glm::length(bsCenter - bsCenterProjection) < bsRadius) {
                            minDistance = std::abs(bsCenterProjectionDistance);
                            pickedBrick = brick.get();
                        }
                    }
                }
            }
        }

        return pickedBrick;
    }

    void showPointedBrick(GameState& gameState)
    {
        // Reset color for all bricks first
        for (auto& brick : gameState.bricks) {
            brick->apparentColor(brick->color());
        }

        // @fixme Green is ugly, would love better highlight system that does not hide previous color (halo?)
        if (gameState.pointedBrick != nullptr) {
            gameState.pointedBrick->apparentColor({0, 1, 0});
        }
    }

    void onUpdateVr(GameState& gameState)
    {
        auto& engine = *gameState.engine;
        if (!engine.vrDeviceValid(VrDeviceType::RightHand)) return;

        // Let the ray mesh follow the hand
        auto handTransform = engine.vrDeviceTransform(VrDeviceType::RightHand);
        gameState.rayPickingEntity->get<sill::TransformComponent>().worldTransform(handTransform);

        // Check if any brick is hit
        Ray ray;
        ray.origin = handTransform[3];
        ray.direction = glm::normalize(handTransform * glm::vec4{0, 0, -1, 0});
        gameState.pointedBrick = pickBrick(gameState, ray);

        showPointedBrick(gameState);
    }

    void onUpdateMouse(GameState& gameState)
    {
        // Checking if a brick is under cursor
        gameState.pointedBrick = pickBrick(gameState, gameState.pickingRay);

        showPointedBrick(gameState);
    }
}

void rayPickingEnabled(GameState& gameState, bool enabled)
{
    g_rayPickingEnabled = enabled;

    if (enabled) {
        gameState.rayPickingEntity->get<sill::TransformComponent>().scaling({1, 1, 1});
    }
    else {
        gameState.pointedBrick = nullptr;
        gameState.rayPickingEntity->get<sill::TransformComponent>().scaling({0, 0, 0});
        showPointedBrick(gameState);
    }
}

void setupRayPicking(GameState& gameState)
{
    auto& engine = *gameState.engine;

    auto& rayPickingEntity = engine.make<sill::GameEntity>();
    auto& meshComponent = rayPickingEntity.make<sill::MeshComponent>();
    sill::makers::BoxMeshOptions boxMeshOptions;
    boxMeshOptions.origin = sill::BoxOrigin::Bottom;
    // @todo Could be cylinder, and disable shadows
    sill::makers::boxMeshMaker({0.005f, 0.005f, 50.f}, boxMeshOptions)(meshComponent);
    auto& behaviorComponent = rayPickingEntity.make<sill::BehaviorComponent>();
    behaviorComponent.onUpdate([&]() {
        const auto& mouseCoordinates = engine.input().mouseCoordinates();
        gameState.pickingRay = gameState.camera->coordinatesToRay(mouseCoordinates);

        if (!g_rayPickingEnabled) return;

        if (gameState.engine->vrEnabled()) {
            onUpdateVr(gameState);
        }
        else {
            onUpdateMouse(gameState);
        }
    });

    gameState.rayPickingEntity = &rayPickingEntity;
}
