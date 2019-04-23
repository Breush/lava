#pragma once

#include <memory>

namespace lava::sill {
    class GameEngine;
    class IComponent;
}

namespace lava::sill {
    /**
     * A simple game object that can have components.
     */
    class GameEntity final {
    public:
        GameEntity(GameEngine& engine);
        ~GameEntity();

        GameEngine& engine() { return m_engine; }
        const GameEngine& engine() const { return m_engine; }

        /// Check if the specified component exists within the entity.
        template <class ComponentClass>
        bool has() const;
        bool hasComponent(const std::string& hrid) const;

        /// Get the specified component. Does not check if it exists.
        template <class ComponentClass>
        ComponentClass& get();
        IComponent& getComponent(const std::string& hrid);
        template <class ComponentClass>
        const ComponentClass& get() const;
        const IComponent& getComponent(const std::string& hrid) const;

        /// Create a new component on this entity. The entity handles its lifetime.
        template <class ComponentClass, class... Arguments>
        ComponentClass& make(Arguments&&... arguments);

        /// Add a created component to this entity. The entity handles its lifetime.
        void add(const std::string& hrid, std::unique_ptr<IComponent>&& component);

        /// Ensure this entity has the required component. If it does not, make it.
        template <class ComponentClass>
        ComponentClass& ensure();

        /// Remove a previously added (or made) component.
        template <class ComponentClass>
        void remove();
        void removeComponent(const std::string& hrid);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        GameEngine& m_engine;
        Impl* m_impl = nullptr;
    };
}

#include <lava/sill/game-entity.inl>
