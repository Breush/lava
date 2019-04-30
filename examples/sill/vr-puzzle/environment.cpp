#include "./environment.hpp"

#include <iostream>
#include <sstream>

#include "./brick.hpp"
#include "./symbols.hpp"

using namespace lava;

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
    std::vector<sill::GameEntity*> bricks;
    auto brickMaker = sill::makers::glbMeshMaker("./assets/models/vr-puzzle/puzzle-brick.glb");
    for (auto i = 0u; i < 9u; ++i) {
        auto& entity = engine.make<sill::GameEntity>();
        auto& meshComponent = entity.make<sill::MeshComponent>();
        brickMaker(meshComponent);
        bricks.emplace_back(&entity);
    }

    // L3 brick
    {
        auto& entity = engine.make<sill::GameEntity>();
        entity.make<sill::TransformComponent>();
        entity.make<sill::AnimationComponent>();
        // @fixme Make better colliders with some sort of PhysicsComponent
        // Moreover, we can't offset this one. Too bad.
        entity.make<sill::BoxColliderComponent>(glm::vec3{2 * brickExtent.x, 2 * brickExtent.y, brickExtent.z});

        entity.addChild(*bricks[0]);
        entity.addChild(*bricks[1]);
        entity.addChild(*bricks[2]);

        bricks[1]->get<sill::TransformComponent>().translate({brickExtent.x, 0, 0});
        bricks[2]->get<sill::TransformComponent>().translate({0, brickExtent.y, 0});

        gameState.bricks.emplace_back(&entity);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(0, 0);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(1, 0);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(0, 1);
        gameState.bricks.back().blocks = gameState.bricks.back().nonRotatedBlocks;

        for (auto i = 0u; i <= 2u; ++i) {
            bricks[i]->get<sill::MeshComponent>().node(1).mesh->primitive(0).material().set("albedoColor", {0, 1, 1});
        }
    }

    // T4 brick
    {
        auto& entity = engine.make<sill::GameEntity>();
        entity.make<sill::TransformComponent>();
        entity.make<sill::AnimationComponent>();
        entity.make<sill::BoxColliderComponent>(glm::vec3{3 * brickExtent.x, 2 * brickExtent.y, brickExtent.z});

        entity.addChild(*bricks[3]);
        entity.addChild(*bricks[4]);
        entity.addChild(*bricks[5]);
        entity.addChild(*bricks[6]);

        bricks[3]->get<sill::TransformComponent>().translate({-brickExtent.x, 0, 0});
        bricks[5]->get<sill::TransformComponent>().translate({brickExtent.x, 0, 0});
        bricks[6]->get<sill::TransformComponent>().translate({0, brickExtent.y, 0});

        gameState.bricks.emplace_back(&entity);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(0, 0);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(-1, 0);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(1, 0);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(0, 1);
        gameState.bricks.back().blocks = gameState.bricks.back().nonRotatedBlocks;

        for (auto i = 3u; i <= 6u; ++i) {
            bricks[i]->get<sill::MeshComponent>().node(1).mesh->primitive(0).material().set("albedoColor", {1, 1, 0});
        }
    }

    // I2 brick
    {
        auto& entity = engine.make<sill::GameEntity>();
        entity.make<sill::TransformComponent>();
        entity.make<sill::AnimationComponent>();
        entity.make<sill::BoxColliderComponent>(glm::vec3{brickExtent.x, 2 * brickExtent.y, brickExtent.z});

        entity.addChild(*bricks[7]);
        entity.addChild(*bricks[8]);

        bricks[8]->get<sill::TransformComponent>().translate({0, brickExtent.y, 0});

        gameState.bricks.emplace_back(&entity);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(0, 0);
        gameState.bricks.back().nonRotatedBlocks.emplace_back(0, 1);
        gameState.bricks.back().blocks = gameState.bricks.back().nonRotatedBlocks;

        for (auto i = 7u; i <= 8u; ++i) {
            bricks[i]->get<sill::MeshComponent>().node(1).mesh->primitive(0).material().set("albedoColor", {0, 0, 1});
        }
    }
}
