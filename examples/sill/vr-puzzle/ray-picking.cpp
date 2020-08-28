#include "./ray-picking.hpp"

#include <glm/gtx/string_cast.hpp>

using namespace lava;

namespace {
    bool g_rayPickingEnabled = true;

    Brick* pickBrick(GameState& gameState, const Ray& ray)
    {
        Brick* pickedBrick = nullptr;

        float minDistance = 50.f;
        for (auto brick : gameState.level.bricks) {
            if (!brick->userInteractionAllowed()) continue;
            float distance = brick->entity().distanceFrom(ray);
            if (distance > 0.f && distance < minDistance) {
                minDistance = distance;
                pickedBrick = brick;
            }
        }

        if (!pickedBrick) return nullptr;

        // Checking if any barrier is preventing us
        for (auto barrier : gameState.level.barriers) {
            if (barrier->pickingBlocked() &&
                barrier->intersectSegment(gameState.player.position, ray.origin + minDistance * ray.direction)) {
                return nullptr;
            }
        }

        return pickedBrick;
    }

    void showPointedBrick(GameState& gameState)
    {
        // Reset color for all bricks first
        for (auto brick : gameState.level.bricks) {
            brick->selectionHighlighted(false);
        }

        if (gameState.state == State::Idle && gameState.pointedBrick != nullptr) {
            gameState.pointedBrick->selectionHighlighted(true);
        }
    }

    void onUpdateVr(GameState& gameState)
    {
        auto& engine = *gameState.engine;
        if (!engine.vr().deviceValid(VrDeviceType::RightHand)) return;

        // Let the ray mesh follow the hand
        const auto& handTransform = engine.vr().deviceTransform(VrDeviceType::RightHand);
        gameState.rayPickingEntity->get<sill::TransformComponent>().worldTransform(handTransform);

        // Check if any brick is hit
        Ray ray;
        ray.origin = handTransform.translation;
        ray.direction = handTransform.rotation * glm::vec3{0, 0, -1};
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
        gameState.rayPickingEntity->get<sill::TransformComponent>().scaling(1.f);
    }
    else {
        gameState.pointedBrick = nullptr;
        gameState.rayPickingEntity->get<sill::TransformComponent>().scaling(0.f);
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
        meshComponent.primitive(0, 0).shadowsCastable(false);
    }

    auto& behaviorComponent = rayPickingEntity.make<sill::BehaviorComponent>();
    behaviorComponent.onUpdate([&](float /* dt */) {
        const auto& mouseCoordinates = engine.input().mouseCoordinates();
        gameState.pickingRay = gameState.camera.component->unprojectAsRay(mouseCoordinates);

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
