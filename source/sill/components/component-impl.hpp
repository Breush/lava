#pragma once

#include <lava/sill/game-entity.hpp>

namespace lava::sill {
    // Base class for component implementations
    class ComponentImpl {
    public:
        ComponentImpl(GameEntity& entity);
        virtual ~ComponentImpl() = default;

        // IComponent
        virtual void update() = 0;

        // Getters
        GameEntity& entity() { return m_entity; }
        const GameEntity& entity() const { return m_entity; }

    protected:
        // References
        GameEntity& m_entity;
    };
}
