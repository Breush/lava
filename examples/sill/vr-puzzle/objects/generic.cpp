#include "./generic.hpp"

#include "./pedestal.hpp"
#include "../game-state.hpp"

#include <lava/chamber/logger.hpp>

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

    if (kind == "barrier") {
        generic = std::make_unique<Barrier>(gameState);
    }
    else if (kind == "brick") {
        generic = std::make_unique<Brick>(gameState);
    }
    else if (kind == "panel") {
        generic = std::make_unique<Panel>(gameState);
    }
    else if (kind == "pedestal") {
        generic = std::make_unique<Pedestal>(gameState);
    }
    else if (kind.empty()) {
        generic = std::make_unique<Generic>(gameState);
    }
    else {
        chamber::logger.error("vr-puzzle.generic") << "Unknown generic kind '" << kind << "'." << std::endl;
    }

    generic->m_kind = kind;
    return *gameState.level.generics.emplace_back(std::move(generic));
}

// -----

Generic* findGenericByName(GameState& gameState, const std::string& name)
{
    for (const auto& generic : gameState.level.generics) {
        if (generic->name() == name) {
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
