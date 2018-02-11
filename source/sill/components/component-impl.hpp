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
        GameEntity::Impl& entity() { return m_entity; }
        const GameEntity::Impl& entity() const { return m_entity; }

    protected:
        // References
        GameEntity::Impl& m_entity;
    };
}
