#include "./serializer.hpp"

#include <cmath>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <lava/chamber/logger.hpp>
#include <nlohmann/json.hpp>
#include <sstream>

#include "./brick.hpp"

using namespace lava;
using namespace lava::chamber;

namespace {
    glm::mat4 unserializeMat4(const nlohmann::json& json)
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

    nlohmann::json serializeMat4(const glm::mat4& matrix)
    {
        nlohmann::json json = nlohmann::json::array();
        json[0u] = matrix[0u][0u];
        json[1u] = matrix[0u][1u];
        json[2u] = matrix[0u][2u];
        json[3u] = matrix[0u][3u];
        json[4u] = matrix[1u][0u];
        json[5u] = matrix[1u][1u];
        json[6u] = matrix[1u][2u];
        json[7u] = matrix[1u][3u];
        json[8u] = matrix[2u][0u];
        json[9u] = matrix[2u][1u];
        json[10u] = matrix[2u][2u];
        json[11u] = matrix[2u][3u];
        json[12u] = matrix[3u][0u];
        json[13u] = matrix[3u][1u];
        json[14u] = matrix[3u][2u];
        json[15u] = matrix[3u][3u];
        return json;
    }
}

void unserializeLevel(GameState& gameState, const std::string& path)
{
    std::ifstream levelFile(path);
    if (!levelFile.is_open()) {
        logger.error("vr-puzzle") << "Unable to read level file: " << path << "." << std::endl;
        return;
    }

    const auto levelJson = nlohmann::json::parse(levelFile);
    gameState.level.name = levelJson["name"];
    gameState.level.path = path;

    logger.info("vr-puzzle") << "Loading level '" << gameState.level.name << "'..." << std::endl;

    // ----- Entities

    gameState.level.entities.clear();
    for (auto& entityJson : levelJson["entities"]) {
        auto& entity = gameState.engine->make<sill::GameEntity>(entityJson["name"].get<std::string>());
        entity.ensure<sill::TransformComponent>().worldTransform(unserializeMat4(entityJson["transform"]));

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

        gameState.level.entities.emplace_back(&entity);
    }

    // ----- Panels

    gameState.level.panels.clear();
    for (auto& panelJson : levelJson["panels"]) {
        auto panel = std::make_unique<Panel>(gameState);

        panel->transform().worldTransform(unserializeMat4(panelJson["transform"]));

        auto& extentJson = panelJson["extent"];
        glm::uvec2 extent(extentJson[0], extentJson[1]);
        panel->extent(extent);

        gameState.level.panels.emplace_back(std::move(panel));
    }

    // ----- Bricks

    gameState.level.bricks.clear();
    for (auto& brickJson : levelJson["bricks"]) {
        auto brick = std::make_unique<Brick>(gameState);

        std::vector<glm::ivec2> blocks;
        for (auto& blockJson : brickJson["blocks"]) {
            glm::ivec2 block(blockJson[0], blockJson[1]);
            blocks.emplace_back(block);
        }
        brick->blocks(blocks);

        auto& colorJson = brickJson["color"];
        glm::vec3 color(colorJson[0], colorJson[1], colorJson[2]);
        brick->color(color);

        brick->baseRotationLevel(brickJson["rotationLevel"]);

        auto& snapPanelJson = brickJson["snapPanel"];
        if (!snapPanelJson.is_null()) {
            glm::uvec2 snapCoordinates(brickJson["snapCoordinates"][0], brickJson["snapCoordinates"][1]);
            brick->snap(*gameState.level.panels[snapPanelJson], snapCoordinates);
        }

        // @fixme Keep last because of a bug, blocks won't be moved otherwise.
        brick->transform().worldTransform(unserializeMat4(brickJson["transform"]));

        gameState.level.bricks.emplace_back(std::move(brick));
    }
}

void serializeLevel(GameState& gameState, const std::string& path)
{
    logger.info("vr-puzzle") << "Saving level '" << gameState.level.name << "' to " << path << "..." << std::endl;

    nlohmann::json levelJson;

    levelJson["name"] = gameState.level.name;
    levelJson["file"] = {
        {"type", "LEVEL"},
        {"version", 0},
    };

    // ----- Panels

    levelJson["panels"] = nlohmann::json::array();
    for (auto i = 0u; i < gameState.level.panels.size(); ++i) {
        const auto& panel = *gameState.level.panels[i];
        levelJson["panels"][i] = {
            {"transform", serializeMat4(panel.transform().worldTransform())},
            {"extent", nlohmann::json::array({panel.extent().x, panel.extent().y})},
        };
    }

    // ----- Bricks

    levelJson["bricks"] = nlohmann::json::array();
    for (auto i = 0u; i < gameState.level.bricks.size(); ++i) {
        const auto& brick = *gameState.level.bricks[i];

        auto blocks = nlohmann::json::array();
        for (auto block : brick.blocks()) {
            blocks.emplace_back(nlohmann::json::array({block.nonRotatedCoordinates.x, block.nonRotatedCoordinates.y}));
        }

        levelJson["bricks"][i] = {
            {"transform", serializeMat4(brick.transform().worldTransform())},
            {"blocks", blocks},
            {"color", nlohmann::json::array({brick.color().r, brick.color().g, brick.color().b})},
            {"rotationLevel", brick.rotationLevel()},
        };

        uint32_t panelIndex = -1u;
        if (brick.snapped()) {
            for (auto i = 0u; i < gameState.level.panels.size(); ++i) {
                if (gameState.level.panels[i].get() == &brick.snapPanel()) {
                    panelIndex = i;
                    break;
                }
            }
        }

        if (panelIndex != -1u) {
            levelJson["bricks"][i]["snapPanel"] = panelIndex;
            levelJson["bricks"][i]["snapCoordinates"] =
                nlohmann::json::array({brick.snapCoordinates().x, brick.snapCoordinates().y});
        }
        else {
            levelJson["bricks"][i]["snapPanel"] = {}; // null
        }
    }

    // ----- Entities

    levelJson["entities"] = nlohmann::json::array();
    for (auto i = 0u; i < gameState.level.entities.size(); ++i) {
        const auto& entity = *gameState.level.entities[i];
        levelJson["entities"][i] = {
            {"name", entity.name()},
            {"transform", serializeMat4(entity.get<sill::TransformComponent>().worldTransform())},
            {"components", nlohmann::json()},
        };

        auto& componentsJson = levelJson["entities"][i]["components"];
        for (const auto& componentHrid : entity.componentsHrids()) {
            if (componentHrid == "mesh") {
                componentsJson[componentHrid] = {{"path", entity.get<sill::MeshComponent>().path()}};
            }
            else if (componentHrid == "sound-emitter") {
                auto sounds = nlohmann::json::array();
                for (const auto& sound : entity.get<sill::SoundEmitterComponent>().sounds()) {
                    sounds.emplace_back(nlohmann::json({
                        {"hrid", sound.first},
                        {"path", sound.second},
                    }));
                }

                componentsJson[componentHrid] = {{"sounds", sounds}};
            }
            else if (componentHrid == "transform") {
                // Nothing to do
            }
            else {
                logger.warning("vr-puzzle") << "Unhandled component '" << componentHrid << "' to serialize '" << entity.name()
                                            << "' entity." << std::endl;
            }
        }
    }

    // Print out
    std::ofstream levelFile(path);
    levelFile << levelJson.dump(4);
    levelFile.close();

    // Set the newly saved path to what we're working on.
    gameState.level.path = path;
}
