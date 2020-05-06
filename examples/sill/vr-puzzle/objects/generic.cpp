#include "./generic.hpp"

#include "./pedestal.hpp"
#include "../game-state.hpp"

using namespace lava;

Generic::Generic(GameState& gameState)
    : Object(gameState)
{
    m_entity = &gameState.engine->make<sill::GameEntity>("generic");
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

Generic& Generic::make(GameState& gameState, const std::string& kind)
{
    std::unique_ptr<Generic> generic;
    if (kind == "pedestal") {
        generic = std::make_unique<Pedestal>(gameState);
    }
    else {
        generic = std::make_unique<Generic>(gameState);
    }

    generic->m_kind = kind;
    return *gameState.level.generics.emplace_back(std::move(generic));
}

// -----

Generic* findGenericByName(GameState& gameState, const std::string& name)
{
    for (const auto& generic : gameState.level.generics) {
        if (generic->entity().name() == name) {
            return generic.get();
        }
    }

    return nullptr;
}

uint32_t findGenericIndex(GameState& gameState, const sill::GameEntity& entity)
{
    for (auto i = 0u; i < gameState.level.generics.size(); ++i) {
        if (&gameState.level.generics[i]->entity() == &entity) {
            return i;
        }
    }

    return -1u;
}
