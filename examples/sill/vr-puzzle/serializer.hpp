#pragma once

#include "./game-state.hpp"

#include <nlohmann/json.hpp>

void unserializeLevel(GameState& gameState, const std::string& path);
void serializeLevel(GameState& gameState, const std::string& path);

std::unique_ptr<Brick> unserializeBrick(GameState& gameState, const nlohmann::json& json);
nlohmann::json serialize(GameState& gameState, const Brick& brick);

std::unique_ptr<Panel> unserializePanel(GameState& gameState, const nlohmann::json& json);
nlohmann::json serialize(GameState& gameState, const Panel& panel);

std::unique_ptr<Barrier> unserializeBarrier(GameState& gameState, const nlohmann::json& json);
nlohmann::json serialize(GameState& gameState, const Barrier& barrier);

lava::sill::GameEntity& unserializeEntity(GameState& gameState, const nlohmann::json& json);
nlohmann::json serialize(GameState& gameState, const lava::sill::GameEntity& entity);
