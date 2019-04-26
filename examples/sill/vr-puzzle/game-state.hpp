#pragma once

#include <lava/sill.hpp>
#include <unordered_map>

struct GameState {
    lava::sill::GameEngine* engine = nullptr;
    std::vector<lava::sill::GameEntity*> draggableEntities; // Bricks and such

    std::vector<std::vector<const lava::sill::MeshNode*>> tableBindingNodes;
};
