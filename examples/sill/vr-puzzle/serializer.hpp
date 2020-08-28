#pragma once

#include <glm/glm.hpp>
#include <lava/core/transform.hpp>
#include <nlohmann/json.hpp>

struct GameState;
class Object;

void unserializeLevel(GameState& gameState, const std::string& path);
void serializeLevel(GameState& gameState, const std::string& path);

Object& duplicateBySerialization(GameState& gameState, const Object& object);

glm::vec2 unserializeVec2(const nlohmann::json& json);
glm::ivec2 unserializeIvec2(const nlohmann::json& json);
glm::uvec2 unserializeUvec2(const nlohmann::json& json);
glm::vec3 unserializeVec3(const nlohmann::json& json);
glm::quat unserializeQuat(const nlohmann::json& json);
lava::Transform unserializeTransform(const nlohmann::json& json);

nlohmann::json serialize(const glm::vec2& vector);
nlohmann::json serialize(const glm::ivec2& vector);
nlohmann::json serialize(const glm::uvec2& vector);
nlohmann::json serialize(const glm::vec3& vector);
nlohmann::json serialize(const lava::Transform& transform);
