#include "./generic.hpp"

#include "../game-state.hpp"

using namespace lava;

Generic::Generic(GameState& gameState)
    : Object(gameState)
{
    gameState.level.generics.emplace_back(this);
}

void Generic::clear(bool removeFromLevel)
{
    if (removeFromLevel) {
        auto genericIt = std::find(m_gameState.level.generics.begin(), m_gameState.level.generics.end(), this);
        m_gameState.level.generics.erase(genericIt);
    }

    // @note Keep last, this destroys us!
    Object::clear(removeFromLevel);
}

void Generic::unserialize(const nlohmann::json& data)
{
    walkable(data["walkable"]);
}

nlohmann::json Generic::serialize() const
{
    nlohmann::json data = {
        {"walkable", m_walkable},
    };

    return data;
}

// -----

Generic* findGenericByName(GameState& gameState, const std::string& name)
{
    for (auto generic : gameState.level.generics) {
        if (generic->name() == name) {
            return generic;
        }
    }

    return nullptr;
}

uint32_t findGenericIndex(GameState& gameState, const sill::GameEntity& entity)
{
    for (auto i = 0u; i < gameState.level.generics.size(); ++i) {
        auto& generic = *gameState.level.generics[i];
        if (&generic.entity() == &entity) {
            return i;
        }
    }

    return -1u;
}
