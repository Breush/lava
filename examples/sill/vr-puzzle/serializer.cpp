#include "./serializer.hpp"

#include <cmath>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <lava/chamber/logger.hpp>
#include <sstream>

#include "./game-state.hpp"

using namespace lava;
using namespace lava::chamber;

glm::vec2 unserializeVec2(const nlohmann::json& json)
{
    return glm::vec2{json[0], json[1]};
}

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

nlohmann::json serialize(const glm::vec2& vector)
{
    return nlohmann::json::array({vector.x, vector.y});
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

// ----- Frames

void unserializeFrame(GameState& /* gameState */, Frame& frame, const nlohmann::json& json)
{
    frame.name(json["name"].get<std::string>());

    auto& entityFrame = frame.entityFrame();

    for (auto& componentJson : json["components"].items()) {
        if (componentJson.key() == "mesh") {
            auto& meshFrameComponent = entityFrame.ensure<sill::MeshFrameComponent>();
            auto path = componentJson.value()["path"].get<std::string>();
            sill::makers::glbMeshMaker(path)(meshFrameComponent);
        }
    }
}

nlohmann::json serialize(GameState& /* gameState */, const Frame& frame)
{
    nlohmann::json json = {
        {"name", frame.name()},
        {"components", nlohmann::json()},
    };

    const auto& entityFrame = frame.entityFrame();

    auto& componentsJson = json["components"];
    for (const auto& componentHrid : entityFrame.componentsHrids()) {
        if (componentHrid == "mesh") {
            const auto& meshComponent = entityFrame.get<sill::MeshFrameComponent>();
            const auto& path = meshComponent.path();
            if (!path.empty()) {
                componentsJson[componentHrid] = {{"path", path}};
            }
        }
        else {
            logger.warning("vr-puzzle") << "Unhandled frame component '" << componentHrid << "'." << std::endl;
        }
    }

    return json;
}

// ----- Objects

void unserializeObject(GameState& gameState, Object& object, const nlohmann::json& json)
{
    object.name(json["name"].get<std::string>());

    auto& entity = object.entity();
    entity.ensure<sill::TransformComponent>().worldTransform(unserializeTransform(json["transform"]));

    if (json.find("frame") != json.end()) {
        auto& entityFrame = gameState.level.frames[json["frame"]]->entityFrame();
        entityFrame.makeEntity(entity);
    }

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

    if (json.find("properties") != json.end()) {
        auto& properties = json["properties"];
        if (properties.find("terrain") != properties.end()) {
            gameState.terrain.entity = &object.entity();
            gameState.terrain.entity->ensure<sill::PhysicsComponent>().dynamic(false);
            gameState.terrain.entity->ensure<sill::ColliderComponent>().addMeshShape();
        }
    }

    object.unserialize(json["data"]);
}

nlohmann::json serialize(GameState& gameState, const Object& object)
{
    const auto& entity = object.entity();

    nlohmann::json json = {
        {"name", object.name()},
        {"transform", serialize(entity.get<sill::TransformComponent>().worldTransform())},
        {"components", nlohmann::json()},
        {"data", object.serialize()},
    };

    if (!object.kind().empty()) {
        json["kind"] = object.kind();
    }

    bool hasFrame = (entity.frame() != nullptr);
    if (hasFrame) {
        json["frame"] = findFrameIndex(gameState, *entity.frame());
    }

    auto& componentsJson = json["components"];
    for (const auto& componentHrid : entity.componentsHrids()) {
        if (componentHrid == "mesh") {
            const auto& meshComponent = entity.get<sill::MeshComponent>();
            const auto& path = meshComponent.path();
            if ((meshComponent.frame() == nullptr) && !path.empty()) {
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
                                        << "' object entity." << std::endl;
        }
    }

    if (gameState.terrain.entity == &object.entity()) {
        json["properties"] = {
            {"terrain", true},
        };
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
    gameState.player.vrAreaPosition = gameState.player.position;

    for (const auto& object : gameState.level.objects) {
        object->clear(false);
    }
    for (const auto& frame : gameState.level.frames) {
        frame->clear(false);
    }

    gameState.level.barriers.clear();
    gameState.level.bricks.clear();
    gameState.level.panels.clear();
    gameState.level.generics.clear();
    gameState.level.objects.clear();
    gameState.level.frames.clear();

    for (auto& frameJson : levelJson["frames"]) {
        auto& frame = Frame::make(gameState);
        unserializeFrame(gameState, frame, frameJson);
    }

    for (auto& objectJson : levelJson["objects"]) {
        auto kind = objectJson["kind"].get<std::string>();
        auto& object = Object::make(gameState, kind);
        unserializeObject(gameState, object, objectJson);
    }

    for (auto& object : gameState.level.objects) {
        object->consolidateReferences();
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

    levelJson["frames"] = nlohmann::json::array();
    for (auto i = 0u; i < gameState.level.frames.size(); ++i) {
        const auto& frame = *gameState.level.frames[i];
        levelJson["frames"][i] = serialize(gameState, frame);
    }

    levelJson["objects"] = nlohmann::json::array();
    for (auto i = 0u; i < gameState.level.objects.size(); ++i) {
        const auto& object = *gameState.level.objects[i];
        levelJson["objects"][i] = serialize(gameState, object);
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
    auto json = serialize(gameState, object);
    auto kind = json["kind"].get<std::string>();
    auto& newObject = Object::make(gameState, kind);
    newObject.mutateBeforeDuplication(json["data"]);
    unserializeObject(gameState, newObject, json);
    newObject.consolidateReferences();
    return newObject;
}
