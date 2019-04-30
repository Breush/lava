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
            const auto bsRadius = std::sqrt(brickExtent.x * brickExtent.x + brickExtent.y * brickExtent.y) / 2.f;

            gameState.pointedBrick = nullptr;
            float minDistance = 50.f;
            for (auto& brick : gameState.bricks) {
                for (auto& simpleBrick : brick.entity->children()) {
                    // Reset color
                    simpleBrick->get<sill::MeshComponent>().node(1).mesh->primitive(0).material().set("albedoColor", brick.color);

                    // @todo For now, this is just a arbitrary ray-sphere detection,
                    // we could do ray-box intersection.
                    glm::vec3 bsCenter = simpleBrick->get<sill::TransformComponent>().worldTransform()[3];
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
                for (auto& simpleBrick : gameState.pointedBrick->entity->children()) {
                    simpleBrick->get<sill::MeshComponent>().node(1).mesh->primitive(0).material().set("albedoColor", {1, 0, 0});
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
        // @todo Currently a new level is just about updating the panel,
        // there is nothing more to load.

        // Setting up level 1
        gameState.panel.extent({3, 3});
        gameState.panel.addLink({0, 0}, {0, 1});
        gameState.panel.addLink({1, 1}, {2, 1});

        // @todo This following behavior should be done in game-logic because it's meant to be done there.
        // @fixme Add a timer before dropping everything!

        // Dropping all bricks
        for (auto& brick : gameState.bricks) {
            brick.snapped = false;
            brick.entity->get<sill::BoxColliderComponent>().enabled(true);
        }
        gameState.panel.updateFromSnappedBricks(gameState);

        // Setting table to unsolved status.
        auto& tableAnimation = gameState.tableEntity->get<sill::AnimationComponent>();
        tableAnimation.start(sill::AnimationFlag::MaterialUniform, *gameState.tableMaterial, "albedoColor", 0.1f);
        tableAnimation.target(sill::AnimationFlag::MaterialUniform, *gameState.tableMaterial, "albedoColor",
                              glm::vec4{1.f, 1.f, 1.f, 1.f});

        return;
    }
    else if (levelId != 0) {
        return;
    }

    auto& engine = *gameState.engine;

    /**
     * Level 0 is composed of:
     *  - 3x3 void panel
     *  - 1 L3 brick
     *  - 1 T4 brick
     *  - 1 I2 brick
     */

    // Load default look for the table panel
    gameState.panel.extent({3, 3});

    // Puzzle individual bricks
    std::vector<sill::GameEntity*> simpleBricks;
    auto brickMaker = sill::makers::glbMeshMaker("./assets/models/vr-puzzle/puzzle-brick.glb");
    for (auto i = 0u; i < 9u; ++i) {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        brickMaker(meshComponent);
        simpleBricks.emplace_back(&entity);
    }

    // L3 brick
    {
        auto& entity = engine.make<sill::GameEntity>();
        entity.make<sill::TransformComponent>();
        entity.make<sill::AnimationComponent>();
        // @fixme Make better colliders with some sort of PhysicsComponent
        // Moreover, we can't offset this one. Too bad.
        entity.make<sill::BoxColliderComponent>(glm::vec3{2 * brickExtent.x, 2 * brickExtent.y, brickExtent.z});

        entity.addChild(*simpleBricks[0]);
        entity.addChild(*simpleBricks[1]);
        entity.addChild(*simpleBricks[2]);

        simpleBricks[1]->get<sill::TransformComponent>().translate({brickExtent.x, 0, 0});
        simpleBricks[2]->get<sill::TransformComponent>().translate({0, brickExtent.y, 0});

        gameState.bricks.emplace_back(&entity);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(0, 0);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(1, 0);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(0, 1);
        gameState.bricks.back().blocks = gameState.bricks.back().nonRotatedBlocks;
        gameState.bricks.back().color = {0, 1, 1};

        for (auto i = 0u; i <= 2u; ++i) {
            simpleBricks[i]->get<sill::MeshComponent>().node(1).mesh->primitive(0).material().set("albedoColor", {0, 1, 1});
        }
    }

    // T4 brick
    {
        auto& entity = engine.make<sill::GameEntity>();
        entity.make<sill::TransformComponent>();
        entity.make<sill::AnimationComponent>();
        entity.make<sill::BoxColliderComponent>(glm::vec3{3 * brickExtent.x, 2 * brickExtent.y, brickExtent.z});

        entity.addChild(*simpleBricks[3]);
        entity.addChild(*simpleBricks[4]);
        entity.addChild(*simpleBricks[5]);
        entity.addChild(*simpleBricks[6]);

        simpleBricks[3]->get<sill::TransformComponent>().translate({-brickExtent.x, 0, 0});
        simpleBricks[5]->get<sill::TransformComponent>().translate({brickExtent.x, 0, 0});
        simpleBricks[6]->get<sill::TransformComponent>().translate({0, brickExtent.y, 0});

        gameState.bricks.emplace_back(&entity);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(0, 0);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(-1, 0);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(1, 0);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(0, 1);
        gameState.bricks.back().blocks = gameState.bricks.back().nonRotatedBlocks;
        gameState.bricks.back().color = {1, 1, 0};

        for (auto i = 3u; i <= 6u; ++i) {
            simpleBricks[i]->get<sill::MeshComponent>().node(1).mesh->primitive(0).material().set("albedoColor", {1, 1, 0});
        }
    }

    // I2 brick
    {
        auto& entity = engine.make<sill::GameEntity>();
        entity.make<sill::TransformComponent>();
        entity.make<sill::AnimationComponent>();
        entity.make<sill::BoxColliderComponent>(glm::vec3{brickExtent.x, 2 * brickExtent.y, brickExtent.z});

        entity.addChild(*simpleBricks[7]);
        entity.addChild(*simpleBricks[8]);

        simpleBricks[8]->get<sill::TransformComponent>().translate({0, brickExtent.y, 0});

        gameState.bricks.emplace_back(&entity);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(0, 0);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(0, 1);
        gameState.bricks.back().blocks = gameState.bricks.back().nonRotatedBlocks;
        gameState.bricks.back().color = {0, 0, 1};

        for (auto i = 7u; i <= 8u; ++i) {
            simpleBricks[i]->get<sill::MeshComponent>().node(1).mesh->primitive(0).material().set("albedoColor", {0, 0, 1});
        }
    }
}
