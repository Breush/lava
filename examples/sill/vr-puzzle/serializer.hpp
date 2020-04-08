#pragma once

#include "./game-state.hpp"

#include <nlohmann/json.hpp>

void unserializeLevel(GameState& gameState, const std::string& path);
void serializeLevel(GameState& gameState, const std::string& path);

Object& duplicateBySerialization(GameState& gameState, const Object& object);
