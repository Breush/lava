#pragma once

#include "./object.hpp"

class Generic : public Object {
public:
    Generic(GameState& gameState);
    void clear(bool removeFromLevel = true) final;
};

uint32_t findGenericIndex(GameState& gameState, const lava::sill::GameEntity& entity);
