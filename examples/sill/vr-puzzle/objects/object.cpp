#include "./object.hpp"

#include "../game-state.hpp"

#include <lava/chamber/logger.hpp>

using namespace lava;

Object::Object(GameState& gameState)
    : m_gameState(gameState)
{
    m_entity = &gameState.engine->make<sill::Entity>("object");
}

void Object::clear(bool removeFromLevel)
{
    if (m_entity != nullptr) {
        m_gameState.engine->remove(*m_entity);
    }

    if (removeFromLevel) {
        auto objectIt = std::find_if(m_gameState.level.objects.begin(), m_gameState.level.objects.end(), [this](const std::unique_ptr<Object>& object) {
            return (object.get() == this);
        });
        m_gameState.level.objects.erase(objectIt);
    }
}

Object& Object::make(GameState& gameState, const std::string& kind)
{
    std::unique_ptr<Object> object;

    if (kind == "barrier") {
        object = std::make_unique<Barrier>(gameState);
    }
    else if (kind == "brick") {
        object = std::make_unique<Brick>(gameState);
    }
    else if (kind == "panel") {
        object = std::make_unique<Panel>(gameState);
    }
    else if (kind == "pedestal") {
        object = std::make_unique<Pedestal>(gameState);
    }
    else if (kind == "generic") {
        object = std::make_unique<Generic>(gameState);
    }
    else {
        chamber::logger.error("vr-puzzle.object") << "Unknown object kind '" << kind << "'." << std::endl;
    }

    object->m_kind = kind;
    return *gameState.level.objects.emplace_back(std::move(object));
}

// -----

Object* findObject(GameState& gameState, const lava::sill::Entity* entity)
{
    if (entity == nullptr) return nullptr;
    return findObject(gameState, *entity);
}

Object* findObject(GameState& gameState, const lava::sill::Entity& entity)
{
    auto pEntity = &entity;

    for (const auto& object : gameState.level.objects) {
        if (&object->entity() == pEntity) {
            return object.get();
        }
    }

    // Climbing up the hierarchy until we find the object.
    while (pEntity->parent() != nullptr) {
        pEntity = pEntity->parent();

        for (const auto& object : gameState.level.objects) {
            if (&object->entity() == pEntity) {
                return object.get();
            }
        }
    }

    return nullptr;
}
