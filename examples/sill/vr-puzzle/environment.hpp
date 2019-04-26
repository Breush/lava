#pragma once

#include "./game-state.hpp"

/// Prepare the environment and load it.
void setupEnvironment(GameState& gameState);

void loadLevel(GameState& gameState, uint32_t levelId);
