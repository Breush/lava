#pragma once

#include "./generic.hpp"

#include <glm/glm.hpp>

/**
 * A barrier prevents user to move bricks outside its limits.
 */
class Barrier final : public Generic {
public:
    Barrier(GameState& gameState);
    void clear(bool removeFromLevel = true) final;

    void unserialize(const nlohmann::json& data);
    nlohmann::json serialize() const;

    float halfSpan() const final { return m_diameter / 2.f; }

    lava::magma::Material& material() { return *mesh().primitive(0u, 0u).material(); }

    /// An un-powered barrier doesn't allow anything to interact or go through.
    bool powered() const { return m_powered; }
    void powered(bool powered);

    float diameter() const { return m_diameter; }
    void diameter(float diameter);

private:
    float m_diameter = 1.f;
    bool m_powered = false;
};

Barrier* findBarrierByName(GameState& gameState, const std::string& name);
Barrier* findBarrier(GameState& gameState, const lava::sill::GameEntity& entity);
uint32_t findBarrierIndex(GameState& gameState, const lava::sill::GameEntity& entity);
