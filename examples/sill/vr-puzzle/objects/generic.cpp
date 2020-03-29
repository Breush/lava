#include "./generic.hpp"

#include "../game-state.hpp"

using namespace lava;

Generic::Generic(GameState& gameState)
    : Object(gameState)
{
}

void Generic::clear(bool removeFromLevel)
{
    if (removeFromLevel) {
        auto genericIndex = findGenericIndex(m_gameState, *m_entity);
        m_gameState.level.generics.erase(m_gameState.level.generics.begin() + genericIndex);
    }

    Object::clear();
}

// -----

uint32_t findGenericIndex(GameState& gameState, const sill::GameEntity& entity)
{
    for (auto i = 0u; i < gameState.level.generics.size(); ++i) {
        if (&gameState.level.generics[i]->entity() == &entity) {
            return i;
        }
    }

    return -1u;
}
