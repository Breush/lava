#include "./generic.hpp"

#include "../game-state.hpp"

using namespace lava;

Generic::Generic(GameState& gameState)
    : Object(gameState)
{
}

void Generic::clear(bool removeFromLevel)
{
    Object::clear(removeFromLevel);

    if (removeFromLevel) {
        auto genericIt = std::find_if(m_gameState.level.generics.begin(), m_gameState.level.generics.end(), [this](const std::unique_ptr<Generic>& generic) {
            return (generic.get() == this);
        });
        m_gameState.level.generics.erase(genericIt);
    }
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
