#include "./environment.hpp"

#include <cmath>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <lava/chamber/logger.hpp>
#include <nlohmann/json.hpp>
#include <sstream>

#include "./brick.hpp"
#include "./symbols.hpp"

using namespace lava;
using namespace lava::chamber;

namespace {
    glm::mat4 parseMat4(const nlohmann::json& json)
    {
        glm::mat4 matrix;
        matrix[0u][0u] = json[0u];
        matrix[0u][1u] = json[1u];
        matrix[0u][2u] = json[2u];
        matrix[0u][3u] = json[3u];
        matrix[1u][0u] = json[4u];
        matrix[1u][1u] = json[5u];
        matrix[1u][2u] = json[6u];
        matrix[1u][3u] = json[7u];
        matrix[2u][0u] = json[8u];
        matrix[2u][1u] = json[9u];
        matrix[2u][2u] = json[10u];
        matrix[2u][3u] = json[11u];
        matrix[3u][0u] = json[12u];
        matrix[3u][1u] = json[13u];
        matrix[3u][2u] = json[14u];
        matrix[3u][3u] = json[15u];
        return matrix;
    }
}

void setupEnvironment(GameState& gameState)
{
    auto& engine = *gameState.engine;

    // Skybox
    {
        engine.environmentTexture("./assets/skies/cloudy/");

        auto& skyboxEntity = engine.make<sill::GameEntity>("skybox");
        auto& skyMeshComponent = skyboxEntity.make<sill::MeshComponent>();
        sill::makers::BoxMeshOptions options{.siding = sill::BoxSiding::In};
        sill::makers::boxMeshMaker(1.f, options)(skyMeshComponent);
        skyMeshComponent.category(RenderCategory::Depthless);

        auto& skyboxMaterial = engine.scene().make<magma::Material>("skybox");
        skyboxMaterial.set("useEnvironmentMap", true);
        skyboxMaterial.set("lod", 1u);
        skyMeshComponent.node(0).mesh->primitive(0).material(skyboxMaterial);
    }

    // Invisible but physically present ground
    {
        auto& entity = engine.make<sill::GameEntity>("ground");
        entity.make<sill::ColliderComponent>();
        entity.get<sill::ColliderComponent>().addInfinitePlaneShape();
        entity.get<sill::PhysicsComponent>().dynamic(false);
    }
}

void loadLevel(GameState& gameState, uint32_t levelId)
{
    gameState.levelId = levelId;

    if (levelId == 0) {
        // Unserializing level
        std::ifstream levelFile("./examples/sill/vr-puzzle/level-intro.json");
        if (!levelFile.is_open()) {
            logger.error("vr-puzzle") << "Unable to read level file." << std::endl;
            return;
        }

        const auto levelJson = nlohmann::json::parse(levelFile);

        logger.info("vr-puzzle") << "Loading level '" << levelJson["name"].get<std::string>() << "'..." << std::endl;

        // ----- Loading entities

        for (auto& entityJson : levelJson["entities"]) {
            auto& entity = gameState.engine->make<sill::GameEntity>(entityJson["name"].get<std::string>());
            entity.ensure<sill::TransformComponent>().worldTransform(parseMat4(entityJson["transform"]));

            for (auto& componentJson : entityJson["components"].items()) {
                if (componentJson.key() == "mesh") {
                    auto& meshComponent = entity.ensure<sill::MeshComponent>();

                    auto path = componentJson.value()["path"].get<std::string>();
                    sill::makers::glbMeshMaker(path)(meshComponent);
                }
                else if (componentJson.key() == "sound-emitter") {
                    auto& soundEmitterComponent = entity.ensure<sill::SoundEmitterComponent>();

                    const auto& sounds = componentJson.value()["sounds"];
                    for (const auto& sound : sounds) {
                        soundEmitterComponent.add(sound["hrid"], sound["path"]);
                    }
                }
                else {
                    logger.warning("vr-puzzle") << "Unhandled component '" << componentJson.key() << "'." << std::endl;
                }
            }
        }

        // ----- Loading bricks

        gameState.bricks.clear();
        for (auto& brickJson : levelJson["bricks"]) {
            auto brick = std::make_unique<Brick>(gameState);
            brick->transform().worldTransform(parseMat4(brickJson["transform"]));

            std::vector<glm::ivec2> blocks;
            for (auto& blockJson : brickJson["blocks"]) {
                glm::ivec2 block(blockJson[0], blockJson[1]);
                blocks.emplace_back(block);
            }
            brick->blocks(blocks);

            auto& colorJson = brickJson["color"];
            glm::vec3 color(colorJson[0], colorJson[1], colorJson[2]);
            brick->color(color);

            gameState.bricks.emplace_back(std::move(brick));
        }

        // ----- Loading panels

        gameState.panels.clear();
        for (auto& panelJson : levelJson["panels"]) {
            auto panel = std::make_unique<Panel>(gameState);

            panel->transform().worldTransform(parseMat4(panelJson["transform"]));

            auto& extentJson = panelJson["extent"];
            glm::uvec2 extent(extentJson[0], extentJson[1]);
            panel->extent(extent);

            gameState.panels.emplace_back(std::move(panel));
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
        gameState.panels[0]->transform().translate({1, 0.8f, 0});
        gameState.panels[0]->transform().rotate({0, 0, 1}, 3.14156f * 0.5f);

        // Open clock
        // @fixme Have panel logic more than level logic.
        auto wakingHall = gameState.engine->findEntityByName("waking-hall");
        wakingHall->get<sill::MeshComponent>().startAnimation("open-clock");
        wakingHall->get<sill::SoundEmitterComponent>().start("open-clock");
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
