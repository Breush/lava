#include "./environment.hpp"

#include <cmath>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <sstream>

#include "./brick.hpp"
#include "./symbols.hpp"

using namespace lava;

namespace {
    // Ray picking
    bool g_rayPickingEnabled = true;
}

void setupEnvironment(GameState& gameState)
{
    auto& engine = *gameState.engine;

    // Ground
    {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::planeMeshMaker({10, 10})(meshComponent);
        entity.make<sill::PlaneColliderComponent>();
    }

    // Panel
    gameState.panel.init(gameState);

    // Raycast
    {
        auto& rayPickingEntity = engine.make<sill::GameEntity>();
        auto& meshComponent = rayPickingEntity.make<sill::MeshComponent>();
        sill::makers::BoxMeshOptions boxMeshOptions;
        boxMeshOptions.origin = sill::BoxOrigin::Bottom;
        // @todo Could be cylinder, and disable shadows
        sill::makers::boxMeshMaker({0.005f, 0.005f, 50.f}, boxMeshOptions)(meshComponent);
        auto& behaviorComponent = rayPickingEntity.make<sill::BehaviorComponent>();
        behaviorComponent.onUpdate([&]() {
            if (!g_rayPickingEnabled) return;

            auto& engine = *gameState.engine;
            if (!engine.vrDeviceValid(VrDeviceType::RightHand)) return;

            auto handTransform = engine.vrDeviceTransform(VrDeviceType::RightHand);
            rayPickingEntity.get<sill::TransformComponent>().worldTransform(handTransform);

            // Check if any brick is hit
            // @fixme Should be done through GameEngine API with raycast colliders?
            glm::vec3 rayOrigin = handTransform[3];
            glm::vec3 rayDirection = glm::normalize(handTransform * glm::vec4{0, 0, 1, 0});
            const auto bsRadius = std::sqrt(blockExtent.x * blockExtent.x + blockExtent.y * blockExtent.y) / 2.f;

            gameState.pointedBrick = nullptr;
            float minDistance = 50.f;
            for (auto& brick : gameState.bricks) {
                for (auto& block : brick.blocks()) {
                    // Reset color
                    // @todo This color override should go through Brick class API.
                    block.entity->get<sill::MeshComponent>().node(1).mesh->primitive(0).material().set("albedoColor",
                                                                                                       brick.color());

                    // @todo For now, this is just a arbitrary ray-sphere detection,
                    // we could do ray-box intersection.
                    glm::vec3 bsCenter = block.entity->get<sill::TransformComponent>().worldTransform()[3];
                    auto rayOriginToBsCenter = bsCenter - rayOrigin;
                    auto bsCenterProjectionDistance = glm::dot(rayDirection, rayOriginToBsCenter);
                    if (bsCenterProjectionDistance < 0.f) {
                        auto bsCenterProjection = rayOrigin + rayDirection * bsCenterProjectionDistance;
                        if (std::abs(bsCenterProjectionDistance) < minDistance) {
                            if (glm::length(bsCenter - bsCenterProjection) < bsRadius) {
                                minDistance = std::abs(bsCenterProjectionDistance);
                                gameState.pointedBrick = &brick;
                            }
                        }
                    }
                }
            }

            // Change color for pointed brick
            // @fixme Red is ugly, would love better highlight system that does not hide previous color (halo?)
            if (gameState.pointedBrick != nullptr) {
                for (auto& block : gameState.pointedBrick->blocks()) {
                    block.entity->get<sill::MeshComponent>().node(1).mesh->primitive(0).material().set("albedoColor", {1, 0, 0});
                }
            }
        });

        gameState.rayPickingEntity = &rayPickingEntity;
    }
}

void rayPickingEnabled(GameState& gameState, bool enabled)
{
    g_rayPickingEnabled = enabled;

    if (enabled) {
        gameState.rayPickingEntity->get<sill::TransformComponent>().scaling({1, 1, 1});
    }
    else {
        gameState.rayPickingEntity->get<sill::TransformComponent>().scaling({0, 0, 0});
    }
}

void loadLevel(GameState& gameState, uint32_t levelId)
{
    gameState.levelId = levelId;

    std::cout << "Loading level " << levelId << std::endl;

    if (levelId == 1) {
        /**
         * Level 0 is composed of:
         *  - 3x3 panel with two links
         *  - 1 L3 brick
         *  - 1 T4 brick
         *  - 1 I2 brick
         */

        gameState.panel.extent({3, 3});
        gameState.panel.addLink({0, 0}, {0, 1});
        gameState.panel.addLink({1, 1}, {2, 1});
        return;
    }
    else if (levelId == 0) {
        /**
         * Level 0 is composed of:
         *  - 3x3 void panel
         *  - 1 L3 brick
         *  - 1 T4 brick
         *  - 1 I2 brick
         */

        // Load default look for the table panel
        gameState.panel.extent({3, 3});

        // L3 brick
        {
            auto& brick = gameState.bricks.emplace_back(gameState);
            brick.blocks({{0, 0}, {1, 0}, {0, 1}});
            brick.color({0, 1, 1});
        }

        // T4 brick
        {
            auto& brick = gameState.bricks.emplace_back(gameState);
            brick.blocks({{0, 0}, {-1, 0}, {1, 0}, {0, 1}});
            brick.color({1, 1, 0});
        }

        // I2 brick
        {
            auto& brick = gameState.bricks.emplace_back(gameState);
            brick.blocks({{0, 0}, {1, 0}});
            brick.color({0, 0, 1});
        }
    }
}
