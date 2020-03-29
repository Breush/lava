#include "./object.hpp"

#include "../game-state.hpp"

#include <iostream>

Object::Object(GameState& gameState)
    : m_gameState(gameState)
{
    gameState.level.objects.emplace_back(this);
}

void Object::clear(bool removeFromLevel)
{
    if (removeFromLevel) {
        auto objectIt = std::find(m_gameState.level.objects.begin(), m_gameState.level.objects.end(), this);
        m_gameState.level.objects.erase(objectIt);
    }

    if (m_entity != nullptr) {
        m_gameState.engine->remove(*m_entity);
    }
}

// -----

Object* findObject(GameState& gameState, const lava::sill::GameEntity* entity)
{
    if (entity == nullptr) return nullptr;
    return findObject(gameState, *entity);
}

Object* findObject(GameState& gameState, const lava::sill::GameEntity& entity)
{
    auto pEntity = &entity;

    // Object are root entities, we go find it.
    while (pEntity->parent() != nullptr) {
        pEntity = pEntity->parent();
    }

    for (auto object : gameState.level.objects) {
        if (&object->entity() == pEntity) {
            return object;
        }
    }

    return nullptr;
}
