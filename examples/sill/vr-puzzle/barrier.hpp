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

    const lava::magma::Material& material() const { return *m_entity->get<lava::sill::MeshComponent>().primitive(0u, 0u).material(); }
    lava::magma::Material& material() { return *m_entity->get<lava::sill::MeshComponent>().primitive(0u, 0u).material(); }
    const lava::sill::AnimationComponent& animation() const { return m_entity->get<lava::sill::AnimationComponent>(); }
    lava::sill::AnimationComponent& animation() { return m_entity->get<lava::sill::AnimationComponent>(); }
    const lava::sill::TransformComponent& transform() const { return m_entity->get<lava::sill::TransformComponent>(); }
    lava::sill::TransformComponent& transform() { return m_entity->get<lava::sill::TransformComponent>(); }

    const std::string& name() const { return m_name; }
    void name(const std::string& name) { m_name = name; }

    /// An un-powered barrier doesn't allow anything to interact or go through.
    bool powered() const { return m_powered; }
    void powered(bool powered);

    float diameter() const { return m_diameter; }
    void diameter(float diameter);

private:
    GameState& m_gameState;
    lava::sill::GameEntity* m_entity = nullptr;

    std::string m_name;
    float m_diameter = 1.f;
    bool m_powered = false;
};

Barrier* findBarrierByName(GameState& gameState, const std::string& name);
Barrier* findBarrier(GameState& gameState, const lava::sill::GameEntity& entity);
uint32_t findBarrierIndex(GameState& gameState, const lava::sill::GameEntity& entity);
