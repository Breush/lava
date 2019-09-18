#pragma once

#include "./game-state.hpp"

/// Prepare the environment and load it.
void setupEnvironment(GameState& gameState);

// Levels
void levelSolved(GameState& gameState);
void loadLevel(GameState& gameState, const std::string& levelPath);
