#pragma once

#include "./object.hpp"

// A generic class only holding a simple mesh.
class Generic : public Object {
public:
    Generic(GameState& gameState);
    virtual void clear(bool removeFromLevel = true);

    void unserialize(const nlohmann::json& data) final;
    nlohmann::json serialize() const final;

    bool walkable() const { return m_walkable; }
    void walkable(bool walkable) { m_walkable = walkable; }

private:
    bool m_walkable = false;
};

Generic* findGenericByName(GameState& gameState, const std::string& name);
uint32_t findGenericIndex(GameState& gameState, const lava::sill::Entity& entity);
