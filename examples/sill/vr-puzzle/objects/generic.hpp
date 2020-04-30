#pragma once

#include "./object.hpp"

class Generic : public Object {
public:
    Generic(GameState& gameState);
    void clear(bool removeFromLevel = true) final;

    bool walkable() const { return m_walkable; }
    void walkable(bool walkable) { m_walkable = walkable; }

private:
    bool m_walkable = true;
};

Generic* findGenericByName(GameState& gameState, const std::string& name);
uint32_t findGenericIndex(GameState& gameState, const lava::sill::GameEntity& entity);
