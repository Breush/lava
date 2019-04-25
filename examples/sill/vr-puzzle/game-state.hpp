#pragma once

#include <lava/sill.hpp>

struct GameState {
    lava::sill::GameEngine* engine = nullptr;
    std::vector<lava::sill::GameEntity*> draggableEntities; // Bricks and such
};
