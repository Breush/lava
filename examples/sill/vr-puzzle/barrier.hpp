#pragma once

#include <glm/glm.hpp>
#include <lava/sill.hpp>

struct GameState;

/**
 * A barrier prevents user to move bricks outside its limits.
 */
class Barrier {
public:
    Barrier(GameState& gameState);
    ~Barrier();

    const lava::sill::GameEntity& entity() const { return *m_entity; }
    lava::sill::GameEntity& entity() { return *m_entity; }

    const lava::sill::TransformComponent& transform() const { return m_entity->get<lava::sill::TransformComponent>(); }
    lava::sill::TransformComponent& transform() { return m_entity->get<lava::sill::TransformComponent>(); }

private:
    GameState& m_gameState;
    lava::sill::GameEntity* m_entity = nullptr;
};
