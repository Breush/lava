#pragma once

#include "./object.hpp"

#include <glm/glm.hpp>

/**
 * A barrier prevents user to move bricks outside its limits.
 */
class Barrier : public Object {
public:
    Barrier(GameState& gameState);
    void clear(bool removeFromLevel = true) final;
    float halfSpan() const final { return m_diameter / 2.f; }

    lava::magma::Material& material() { return *mesh().primitive(0u, 0u).material(); }

    const std::string& name() const { return m_name; }
    void name(const std::string& name) { m_name = name; }

    /// An un-powered barrier doesn't allow anything to interact or go through.
    bool powered() const { return m_powered; }
    void powered(bool powered);

    float diameter() const { return m_diameter; }
    void diameter(float diameter);

private:
    std::string m_name;
    float m_diameter = 1.f;
    bool m_powered = false;
};

Barrier* findBarrierByName(GameState& gameState, const std::string& name);
Barrier* findBarrier(GameState& gameState, const lava::sill::GameEntity& entity);
uint32_t findBarrierIndex(GameState& gameState, const lava::sill::GameEntity& entity);
