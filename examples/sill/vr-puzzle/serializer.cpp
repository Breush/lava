#include "./serializer.hpp"

#include <cmath>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <lava/chamber/logger.hpp>
#include <nlohmann/json.hpp>
#include <sstream>

#include "./game-state.hpp"

using namespace lava;
using namespace lava::chamber;

namespace {
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

    nlohmann::json serialize(const lava::Transform& transform)
    {
        nlohmann::json json;
        json["translation"] = serialize(transform.translation);
        json["rotation"] = serialize(transform.rotation);
        json["scaling"] = transform.scaling;
        return json;
    }
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

// ----- Panels

void unserializePanel(Panel& panel, GameState& gameState, const nlohmann::json& json)
{
    panel.name(json["name"]);
    panel.extent(unserializeUvec2(json["extent"]));

    auto& barriersJson = json["barriers"];
    for (auto& barrierJson : barriersJson) {
        panel.addBarrier(*gameState.level.barriers[barrierJson]);
    }

    panel.transform().worldTransform(unserializeTransform(json["transform"]));
}

nlohmann::json serialize(GameState& gameState, const Panel& panel)
{
    nlohmann::json json = {
        {"name", panel.name()},
        {"transform", serialize(panel.transform().worldTransform())},
        {"extent", serialize(panel.extent())},
        {"barriers", nlohmann::json::array()},
    };

    for (const auto& barrier : panel.barriers()) {
        json["barriers"].emplace_back(findBarrierIndex(gameState, barrier->entity()));
    }

    return json;
}

// ----- Barriers

void unserializeBarrier(Barrier& barrier, GameState& /* gameState */, const nlohmann::json& json)
{
    barrier.name(json["name"]);
    barrier.transform().worldTransform(unserializeTransform(json["transform"]));
    barrier.diameter(json["diameter"]);
    barrier.powered(json["powered"]);
}

nlohmann::json serialize(GameState& /* gameState */, const Barrier& barrier)
{
    nlohmann::json json = {
        {"name", barrier.name()},
        {"transform", serialize(barrier.transform().worldTransform())},
        {"powered", barrier.powered()},
        {"diameter", barrier.diameter()},
    };

    return json;
}

// ----- Generics

void unserializeGeneric(Generic& generic, GameState& gameState, const nlohmann::json& json)
{
    auto& entity = gameState.engine->make<sill::GameEntity>(json["name"].get<std::string>());
    entity.ensure<sill::TransformComponent>().worldTransform(unserializeTransform(json["transform"]));

    if (json.find("data") != json.end()) {
        generic.unserialize(json["data"]);
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
        else if (componentJson.key() == "collider") {
            auto& colliderComponent = entity.ensure<sill::ColliderComponent>();
            auto& meshComponent = entity.ensure<sill::MeshComponent>();
            const auto& boxShapes = componentJson.value()["boxShapes"];
            for (const auto& boxShape : boxShapes) {
                auto offset = unserializeVec3(boxShape["offset"]);
                auto extent = unserializeVec3(boxShape["extent"]);
                colliderComponent.addBoxShape(offset, extent);

                // @todo This looks like a dirty way of doing that.
                // We should be able to serialize that it was made from specific a mesh maker...
                sill::makers::boxMeshMaker(extent)(meshComponent);
                meshComponent.category(RenderCategory::Translucent);
                meshComponent.enabled(gameState.state == State::Editor);
                meshComponent.primitive(0u, 0u).material(gameState.editor.resources.colliderMaterial);
            }

            generic.walkable(json["walkable"]);
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

    generic.entity(entity);
}

nlohmann::json serialize(GameState& /* gameState */, const Generic& generic)
{
    const auto& entity = generic.entity();

    nlohmann::json json = {
        {"name", entity.name()},
        {"transform", serialize(entity.get<sill::TransformComponent>().worldTransform())},
        {"components", nlohmann::json()},
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
        else if (componentHrid == "collider") {
            const auto& colliderComponent = entity.get<sill::ColliderComponent>();
            auto boxShapes = nlohmann::json::array();
            for (const auto& boxShape : colliderComponent.boxShapes()) {
                boxShapes.emplace_back(nlohmann::json({
                    {"offset", serialize(boxShape.offset)},
                    {"extent", serialize(boxShape.extent)},
                }));
            }
            componentsJson[componentHrid] = {{"boxShapes", boxShapes}};

            json["walkable"] = generic.walkable();
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
    gameState.level.panels.clear();
    gameState.level.bricks.clear();
    gameState.level.generics.clear();
    gameState.level.objects.clear();

    // @note We add the object on the list before unserializing it.
    // Because, for example, in Panel::updateFromSnappedBricks called by Brick,
    // we go through gameState.level.bricks. So the brick has to be added
    // to the list beforehands.

    for (auto& barrierJson : levelJson["barriers"]) {
        auto& barrier = gameState.level.barriers.emplace_back(std::make_unique<Barrier>(gameState));
        unserializeBarrier(*barrier, gameState, barrierJson);
    }
    for (auto& panelJson : levelJson["panels"]) {
        auto& panel = gameState.level.panels.emplace_back(std::make_unique<Panel>(gameState));
        unserializePanel(*panel, gameState, panelJson);
    }
    for (const auto& brickJson : levelJson["bricks"]) {
        auto& brick = gameState.level.bricks.emplace_back(std::make_unique<Brick>(gameState));
        unserializeBrick(*brick, gameState, brickJson);
    }
    for (auto& genericJson : levelJson["generics"]) {
        auto kind = (genericJson.find("kind") == genericJson.end()) ? std::string() : genericJson["kind"].get<std::string>();
        auto& generic = Generic::make(gameState, kind);
        unserializeGeneric(generic, gameState, genericJson);
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

    levelJson["barriers"] = nlohmann::json::array();
    for (auto i = 0u; i < gameState.level.barriers.size(); ++i) {
        const auto& barrier = *gameState.level.barriers[i];
        levelJson["barriers"][i] = serialize(gameState, barrier);
    }

    levelJson["panels"] = nlohmann::json::array();
    for (auto i = 0u; i < gameState.level.panels.size(); ++i) {
        const auto& panel = *gameState.level.panels[i];
        levelJson["panels"][i] = serialize(gameState, panel);
    }

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
    else if (entity.name() == "panel") {
        auto json = serialize(gameState, dynamic_cast<const Panel&>(object));
        auto& panel = *gameState.level.panels.emplace_back(std::make_unique<Panel>(gameState));
        unserializePanel(panel, gameState, json);
        return panel;
    }
    else if (entity.name() == "barrier") {
        auto json = serialize(gameState, dynamic_cast<const Barrier&>(object));
        auto& barrier = *gameState.level.barriers.emplace_back(std::make_unique<Barrier>(gameState));
        unserializeBarrier(barrier, gameState, json);
        return barrier;
    }

    // Generics
    auto json = serialize(gameState, dynamic_cast<const Generic&>(object));
    auto kind = (json.find("kind") == json.end()) ? std::string() : json["kind"].get<std::string>();
    auto& generic = Generic::make(gameState, kind);
    unserializeGeneric(generic, gameState, json);
    generic.consolidateReferences();
    return generic;
}
