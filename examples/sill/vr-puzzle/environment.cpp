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
                for (auto& block : brick->blocks()) {
                    // Reset color
                    // @todo This color override should go through Brick class API.
                    block.entity->get<sill::MeshComponent>().node(1).mesh->primitive(0).material().set("albedoColor",
                                                                                                       brick->color());

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
                                gameState.pointedBrick = brick.get();
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

    if (levelId == 0) {
        /**
         *  - 3x3 void panel
         *      - 1 L3 brick
         *      - 1 T4 brick
         *      - 1 I2 brick
         */
        gameState.panels.resize(1);

        gameState.panels[0] = std::make_unique<Panel>(gameState);
        gameState.panels[0]->transform().worldTransform(glm::rotate(glm::mat4(1.f), 3.14156f, {0, 0, 1}));
        gameState.panels[0]->extent({3, 3});

        gameState.bricks.clear();

        // L3 brick
        {
            auto brick = std::make_unique<Brick>(gameState);
            brick->blocks({{0, 0}, {1, 0}, {0, 1}});
            brick->color({0, 1, 1});
            gameState.bricks.emplace_back(std::move(brick));
        }

        // T4 brick
        {
            auto brick = std::make_unique<Brick>(gameState);
            brick->blocks({{0, 0}, {-1, 0}, {1, 0}, {0, 1}});
            brick->color({1, 1, 0});
            gameState.bricks.emplace_back(std::move(brick));
        }

        // I2 brick
        {
            auto brick = std::make_unique<Brick>(gameState);
            brick->blocks({{0, 0}, {0, 1}});
            brick->color({0, 0, 1});
            gameState.bricks.emplace_back(std::move(brick));
        }
    }
    else if (levelId == 1) {
        /**
         *  - 3x3 panel with two links
         *      - 1 L3 brick
         *      - 1 T4 brick
         *      - 1 I2 brick
         */
        gameState.panels.resize(1);

        gameState.panels[0] = std::make_unique<Panel>(gameState);
        gameState.panels[0]->transform().worldTransform(glm::rotate(glm::mat4(1.f), 3.14156f, {0, 0, 1}));
        gameState.panels[0]->extent({3, 3});
        gameState.panels[0]->addLink({0, 0}, {0, 1});
        gameState.panels[0]->addLink({1, 1}, {2, 1});
    }
    else if (levelId == 2) {
        /**
         *  - 3x3 void panel
         *      - 1 I3 brick
         *      - 3 I2 bricks
         *  - 3x3 void panel
         *      - 3 L3 bricks
         *
         * @note The second panel is unsolvable by itself,
         * and it will be the only panel the user see at first sight.
         * The other one being behind him.
         */
        gameState.panels.resize(2);

        gameState.panels[0] = std::make_unique<Panel>(gameState);
        glm::mat4 transform = glm::mat4(1.f);
        transform = glm::translate(transform, {1.f, 0.f, 0.f});
        transform = glm::rotate(transform, 3.14156f, {0, 0, 1});
        gameState.panels[0]->transform().worldTransform(glm::rotate(glm::mat4(1.f), 3.14156f, {0, 0, 1}));
        gameState.panels[0]->animation().start(sill::AnimationFlag::WorldTransform, 2.f);
        gameState.panels[0]->animation().target(sill::AnimationFlag::WorldTransform, transform);
        gameState.panels[0]->extent({3, 3});

        gameState.panels[1] = std::make_unique<Panel>(gameState);
        gameState.panels[1]->transform().worldTransform(glm::translate(glm::mat4(1.f), {-1.f, 0.f, 0.f}));
        gameState.panels[1]->extent({3, 3});

        gameState.bricks.clear();

        // I2 bricks
        for (auto i = 0u; i <= 2u; ++i) {
            auto brick = std::make_unique<Brick>(gameState);
            brick->blocks({{0, 0}, {0, 1}});
            brick->color({0, 0, 1});
            brick->transform().translate({-0.8f, 0.f, 0.f});
            gameState.bricks.emplace_back(std::move(brick));
        }

        // I3 brick
        {
            auto brick = std::make_unique<Brick>(gameState);
            brick->blocks({{0, 0}, {0, 1}, {0, 2}});
            brick->color({0, 1, 1});
            brick->transform().translate({-0.8f, 0.f, 0.f});
            gameState.bricks.emplace_back(std::move(brick));
        }

        // L3 bricks
        for (auto i = 0u; i <= 2u; ++i) {
            auto brick = std::make_unique<Brick>(gameState);
            brick->blocks({{0, 0}, {1, 0}, {0, 1}});
            brick->color({1, 1, 0});
            brick->transform().translate({0.8f, 0.f, 0.f});
            gameState.bricks.emplace_back(std::move(brick));
        }
    }
}
