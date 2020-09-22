#pragma once

#include <lava/sill/entity.hpp>

namespace lava::sill {
    // @fixme Sound useless, right?
    // Base class for component implementations
    class ComponentImpl {
    public:
        ComponentImpl(Entity& entity);
        virtual ~ComponentImpl() = default;

        // Getters
        Entity& entity() { return m_entity; }
        const Entity& entity() const { return m_entity; }

    protected:
        // References
        Entity& m_entity;
    };
}
