#pragma once

#include "./game-state.hpp"

/// Prepare the camera and all actions.
void setupCamera(GameState& gameState);

/// Change the camera behavior.
void setCameraMode(GameState& gameState, CameraMode mode);
