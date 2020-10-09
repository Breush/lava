#pragma once

#include "./game-state.hpp"

void setupEditor(GameState& gameState);

void unselectAllObjects(GameState& gameState, bool signalSelectionChanged = true);
void selectObject(GameState& gameState, Object* object, bool addToSelection, bool signalSelectionChanged = true);
