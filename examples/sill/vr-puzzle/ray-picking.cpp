#include "./ray-picking.hpp"

#include <glm/gtx/string_cast.hpp>
#include <iostream>

using namespace lava;

namespace {
    bool g_rayPickingEnabled = true;

    Brick* pickBrick(GameState& gameState, const Ray& ray)
    {
        Brick* pickedBrick = nullptr;

        float minDistance = 50.f;
        for (auto& brick : gameState.level.bricks) {
            for (auto& block : brick->blocks()) {
                float distance = block.entity->distanceFrom(ray);
                if (distance > 0.f && distance < minDistance) {
                    minDistance = distance;
                    pickedBrick = brick.get();
                }
            }
        }

        return pickedBrick;
    }

    void showPointedBrick(GameState& gameState)
    {
        // Reset color for all bricks first
        for (auto& brick : gameState.level.bricks) {
            brick->apparentColor(brick->color());
        }

        // @fixme Green is ugly, would love better highlight system that does not hide previous color (halo?)
        if (gameState.state == State::Idle && gameState.pointedBrick != nullptr) {
            gameState.pointedBrick->apparentColor({0, 1, 0});
        }
    }

    void onUpdateVr(GameState& gameState)
    {
        auto& engine = *gameState.engine;
        if (!engine.vr().deviceValid(VrDeviceType::RightHand)) return;

        // Let the ray mesh follow the hand
        auto handTransform = engine.vr().deviceTransform(VrDeviceType::RightHand);
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

    auto& rayPickingEntity = engine.make<sill::GameEntity>("ray-picking");
    rayPickingEntity.ensure<sill::TransformComponent>();

    if (gameState.engine->vr().enabled()) {
        auto& meshComponent = rayPickingEntity.make<sill::MeshComponent>();

        sill::makers::BoxMeshOptions boxMeshOptions;
        boxMeshOptions.origin = sill::BoxOrigin::Bottom;
        // @todo Could be cylinder, and disable shadows
        sill::makers::boxMeshMaker({0.005f, 0.005f, 50.f}, boxMeshOptions)(meshComponent);
        meshComponent.node(0).mesh->primitive(0).shadowsCastable(false);
    }

    auto& behaviorComponent = rayPickingEntity.make<sill::BehaviorComponent>();
    behaviorComponent.onUpdate([&](float /* dt */) {
        const auto& mouseCoordinates = engine.input().mouseCoordinates();
        gameState.pickingRay = gameState.camera->coordinatesToRay(mouseCoordinates);

        if (!g_rayPickingEnabled) return;

        if (gameState.engine->vr().enabled()) {
            onUpdateVr(gameState);
        }
        else {
            onUpdateMouse(gameState);
        }
    });

    gameState.rayPickingEntity = &rayPickingEntity;
}
