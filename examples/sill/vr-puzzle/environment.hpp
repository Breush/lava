#pragma once

#include "./game-state.hpp"

/// Prepare the environment and load it.
void setupEnvironment(GameState& gameState);

// Levels
void levelSolved(GameState& gameState);
void loadLevel(GameState& gameState, const std::string& levelPath);

// Substance
void freeSubstance(GameState& gameState, const std::string& substanceName, float ghostFactor = 0.f);
void revealSubstance(GameState& gameState, const std::string& substanceName, float ghostFactor = 0.f);

// Terrain
float distanceToTerrain(GameState& gameState, const lava::Ray& ray, Generic** pGeneric = nullptr, float maxDistance = 1000.f);
