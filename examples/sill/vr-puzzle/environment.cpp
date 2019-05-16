#include "./environment.hpp"

#include <cmath>
#include <glm/gtx/string_cast.hpp>
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
        entity.make<sill::ColliderComponent>();
        entity.get<sill::ColliderComponent>().addInfinitePlaneShape();
        entity.get<sill::PhysicsComponent>().dynamic(false);
    }
}

void loadLevel(GameState& gameState, uint32_t levelId)
{
    gameState.levelId = levelId;

    std::cout << "Loading level " << levelId << std::endl;

    if (levelId == 0) {
        // Waking hall
        {
            auto& entity = gameState.engine->make<sill::GameEntity>();
            auto& meshComponent = entity.make<sill::MeshComponent>();
            sill::makers::glbMeshMaker("./assets/models/vr-puzzle/waking-hall.glb")(meshComponent);
            entity.get<sill::TransformComponent>().rotate({0, 0, 1}, 3.14156f * 0.5f);
            entity.ensure<sill::SoundEmitterComponent>().add("open-clock", "./assets/sounds/vr-puzzle/open-clock.wav");
            gameState.wakingHall = &entity;
        }

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
        gameState.panels[0]->transform().translate({-1, -0.8f, 0});
        gameState.panels[0]->transform().rotate({0, 0, 1}, 3.14156f * 0.5f);

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
        gameState.panels[0]->transform().translate({-1, -0.8f, 0});
        gameState.panels[0]->transform().rotate({0, 0, 1}, 3.14156f * 0.5f);

        // Open clock
        // @fixme Have panel logic more than level logic.
        gameState.wakingHall->get<sill::MeshComponent>().startAnimation("open-clock");
        gameState.wakingHall->get<sill::SoundEmitterComponent>().start("open-clock");
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
