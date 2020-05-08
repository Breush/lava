#include "./serializer.hpp"

#include <cmath>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <lava/chamber/logger.hpp>
#include <sstream>

#include "./game-state.hpp"

using namespace lava;
using namespace lava::chamber;

glm::ivec2 unserializeIvec2(const nlohmann::json& json)
{
    return glm::ivec2{json[0], json[1]};
}

glm::uvec2 unserializeUvec2(const nlohmann::json& json)
{
    return glm::uvec2{json[0], json[1]};
}

glm::vec3 unserializeVec3(const nlohmann::json& json)
{
    return glm::vec3{json[0], json[1], json[2]};
}

glm::quat unserializeQuat(const nlohmann::json& json)
{
    return glm::quat{json[0], json[1], json[2], json[3]};
}

Transform unserializeTransform(const nlohmann::json& json)
{
    Transform transform;
    transform.translation = unserializeVec3(json["translation"]);
    transform.rotation = unserializeQuat(json["rotation"]);
    transform.scaling = json["scaling"];
    return transform;
}

nlohmann::json serialize(const glm::ivec2& vector)
{
    return nlohmann::json::array({vector.x, vector.y});
}

nlohmann::json serialize(const glm::uvec2& vector)
{
    return nlohmann::json::array({vector.x, vector.y});
}

nlohmann::json serialize(const glm::vec3& vector)
{
    return nlohmann::json::array({vector.r, vector.g, vector.b});
}

nlohmann::json serialize(const glm::quat& rotation)
{
    return nlohmann::json::array({rotation.w, rotation.x, rotation.y, rotation.z});
}

nlohmann::json serialize(const Transform& transform)
{
    nlohmann::json json;
    json["translation"] = serialize(transform.translation);
    json["rotation"] = serialize(transform.rotation);
    json["scaling"] = transform.scaling;
    return json;
}

// ----- Bricks

void unserializeBrick(Brick& brick, GameState& gameState, const nlohmann::json& json)
{
    brick.transform().worldTransform(unserializeTransform(json["transform"]));

    std::vector<glm::ivec2> blocks;
    for (auto& blockJson : json["blocks"]) {
        blocks.emplace_back(unserializeIvec2(blockJson));
    }
    brick.blocks(blocks);

    auto& barriersJson = json["barriers"];
    for (auto& barrierJson : barriersJson) {
        brick.addBarrier(*gameState.level.barriers[barrierJson]);
    }

    brick.color(unserializeVec3(json["color"]));
    brick.fixed(json["fixed"]);
    brick.stored(json["stored"]);
    brick.baseRotationLevel(json["rotationLevel"]);

    auto& snapPanelJson = json["snapPanel"];
    if (!snapPanelJson.is_null()) {
        auto& panel = *gameState.level.panels[snapPanelJson];
        brick.snap(panel, unserializeUvec2(json["snapCoordinates"]));
    }
}

nlohmann::json serialize(GameState& gameState, const Brick& brick)
{
    nlohmann::json json = {
        {"transform", serialize(brick.transform().worldTransform())},
        {"blocks", nlohmann::json::array()},
        {"barriers", nlohmann::json::array()},
        {"color", serialize(brick.color())},
        {"fixed", brick.fixed()},
        {"stored", brick.stored()},
        {"rotationLevel", brick.rotationLevel()},
        {"snapPanel", {}},
    };

    for (auto block : brick.blocks()) {
        json["blocks"].emplace_back(serialize(block.nonRotatedCoordinates));
    }

    for (auto barrier : brick.barriers()) {
        json["barriers"].emplace_back(findBarrierIndex(gameState, barrier->entity()));
    }

    if (brick.snapped()) {
        json["snapPanel"] = findPanelIndex(gameState, brick.snapPanel().entity());
        json["snapCoordinates"] = serialize(brick.snapCoordinates());
    }

    return json;
}

// ----- Generics

void unserializeGeneric(Generic& generic, GameState& gameState, const nlohmann::json& json)
{
    generic.name(json["name"].get<std::string>());
    generic.walkable(json["walkable"]);

    auto& entity = generic.entity();
    entity.ensure<sill::TransformComponent>().worldTransform(unserializeTransform(json["transform"]));

    for (auto& componentJson : json["components"].items()) {
        if (componentJson.key() == "mesh") {
            auto& meshComponent = entity.ensure<sill::MeshComponent>();
            auto path = componentJson.value()["path"].get<std::string>();
            sill::makers::glbMeshMaker(path)(meshComponent);
        }
        else if (componentJson.key() == "physics") {
            auto& physicsComponent = entity.ensure<sill::PhysicsComponent>();
            physicsComponent.enabled(componentJson.value()["enabled"].get<bool>());
            physicsComponent.dynamic(componentJson.value()["dynamic"].get<bool>());
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
    if (json.find("data") != json.end()) {
        generic.unserialize(json["data"]);
    }
}

nlohmann::json serialize(GameState& /* gameState */, const Generic& generic)
{
    const auto& entity = generic.entity();

    nlohmann::json json = {
        {"name", generic.name()},
        {"transform", serialize(entity.get<sill::TransformComponent>().worldTransform())},
        {"components", nlohmann::json()},
        {"walkable", generic.walkable()},
    };

    if (!generic.kind().empty()) {
        json["kind"] = generic.kind();
        json["data"] = generic.serialize();
    }

    auto& componentsJson = json["components"];
    for (const auto& componentHrid : entity.componentsHrids()) {
        if (componentHrid == "mesh") {
            const auto& path = entity.get<sill::MeshComponent>().path();
            if (!path.empty()) {
                componentsJson[componentHrid] = {{"path", path}};
            }
        }
        else if (componentHrid == "physics") {
            const auto& physicsComponent = entity.get<sill::PhysicsComponent>();
            componentsJson[componentHrid] = {
                {"enabled", physicsComponent.enabled()},
                {"dynamic", physicsComponent.dynamic()},
            };
        }
        else if (componentHrid == "sound-emitter") {
            auto& soundEmitterComponent = entity.get<sill::SoundEmitterComponent>();
            auto sounds = nlohmann::json::array();
            for (const auto& sound : soundEmitterComponent.sounds()) {
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
        else if (componentHrid == "animation") {
            // Nothing to do
        }
        else if (componentHrid == "behavior") {
            // Nothing to do
        }
        else if (componentHrid == "collider") {
            // Nothing to do
        }
        else {
            logger.warning("vr-puzzle") << "Unhandled component '" << componentHrid << "' to serialize '" << entity.name()
                                        << "' generic entity." << std::endl;
        }
    }

    return json;
}

// ----- Level

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

    const auto playerJson = levelJson["player"];
    gameState.player.position = unserializeVec3(playerJson["position"]);
    gameState.player.direction = unserializeVec3(playerJson["direction"]);

    for (auto object : gameState.level.objects) {
        object->clear(false);
    }

    gameState.level.barriers.clear();
    gameState.level.bricks.clear();
    gameState.level.panels.clear();
    gameState.level.generics.clear();
    gameState.level.objects.clear();

    // @note We add the object on the list before unserializing it.
    // Because, for example, in Panel::updateFromSnappedBricks called by Brick,
    // we go through gameState.level.bricks. So the brick has to be added
    // to the list beforehands.

    for (auto& genericJson : levelJson["generics"]) {
        auto kind = (genericJson.find("kind") == genericJson.end()) ? std::string() : genericJson["kind"].get<std::string>();
        auto& generic = Generic::make(gameState, kind);
        unserializeGeneric(generic, gameState, genericJson);
    }
    for (const auto& brickJson : levelJson["bricks"]) {
        auto& brick = gameState.level.bricks.emplace_back(std::make_unique<Brick>(gameState));
        unserializeBrick(*brick, gameState, brickJson);
    }

    for (auto& generic : gameState.level.generics) {
        generic->consolidateReferences();
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
    levelJson["player"] = {
        {"position", serialize(gameState.player.position)},
        {"direction", serialize(gameState.player.direction)},
    };

    levelJson["bricks"] = nlohmann::json::array();
    for (auto i = 0u; i < gameState.level.bricks.size(); ++i) {
        const auto& brick = *gameState.level.bricks[i];
        levelJson["bricks"][i] = serialize(gameState, brick);
    }

    levelJson["generics"] = nlohmann::json::array();
    for (auto i = 0u; i < gameState.level.generics.size(); ++i) {
        const auto& generic = *gameState.level.generics[i];
        levelJson["generics"][i] = serialize(gameState, generic);
    }

    // Print out
    std::ofstream levelFile(path);
    levelFile << levelJson.dump(4);
    levelFile.close();

    // Set the newly saved path to what we're working on.
    gameState.level.path = path;
}

Object& duplicateBySerialization(GameState& gameState, const Object& object)
{
    auto& entity = object.entity();

    if (entity.name() == "brick") {
        auto json = serialize(gameState, dynamic_cast<const Brick&>(object));
        auto& brick = *gameState.level.bricks.emplace_back(std::make_unique<Brick>(gameState));
        unserializeBrick(brick, gameState, json);
        return brick;
    }

    // Generics
    auto json = serialize(gameState, dynamic_cast<const Generic&>(object));
    auto kind = (json.find("kind") == json.end()) ? std::string() : json["kind"].get<std::string>();
    auto& generic = Generic::make(gameState, kind);
    unserializeGeneric(generic, gameState, json);
    generic.consolidateReferences();
    return generic;
}
