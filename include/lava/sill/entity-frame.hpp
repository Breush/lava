#pragma once

#include <lava/sill/component-holder.hpp>
#include <lava/sill/frame-components/i-frame-component.hpp>

namespace lava::sill {
    class GameEngine;
    class Entity;
}

namespace lava::sill {
    /**
     * Updating the frame will update all entities made from it.
     */
    class EntityFrame final : public ComponentHolder<IFrameComponent> {
    public:
        friend class GameEngine;
        friend class Entity;

    public:
        EntityFrame(GameEngine& engine);
        EntityFrame(const EntityFrame& entityFrame) = delete;

        GameEngine& engine() { return m_engine; }

        std::vector<Entity*>& entities() { return m_entities; }
        const std::vector<Entity*>& entities() const { return m_entities; }

        /**
         * @name Components
         */
        /// @{
        /// Create a new component on this entityFrame.
        template <class ComponentClass, class... Arguments>
        ComponentClass& make(Arguments&&... arguments);

        /// Ensure this entityFrame has the required component. If it does not, make it.
        template <class ComponentClass>
        ComponentClass& ensure();
        /// @}

        /**
         * @name Instancing
         */
        /// @{
        Entity& makeEntity();
        Entity& makeEntity(Entity& entity);
        /// @}

    protected:
        void warnRemoved();
        void warnEntityRemoved(Entity& entity);

    private:
        // References
        GameEngine& m_engine;

        // Attributes
        std::vector<Entity*> m_entities;
    };
}

#include <lava/sill/entity-frame.inl>
