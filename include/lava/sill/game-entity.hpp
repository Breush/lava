#pragma once

#include <lava/sill/pick-precision.hpp>

#include <lava/core/ray.hpp>
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
        GameEntity(GameEngine& engine, const std::string& name);
        GameEntity(const GameEntity& entity) = delete;
        ~GameEntity();

        GameEngine& engine() { return m_engine; }
        const GameEngine& engine() const { return m_engine; }

        /**
         * @name Attributes
         */
        /// @{
        const std::string& name() const;
        void name(const std::string& name);
        /// @}

        /**
         * @name Hierarchy
         */
        /// @{
        /**
         * Parent of the entity.
         *
         * @note Setting the parent WON'T add itself as a child on it.
         */
        GameEntity* parent();
        const GameEntity* parent() const;
        void parent(GameEntity& parent, bool updateParent = true);
        void parent(GameEntity* parent, bool updateParent = true);

        /// Sets an entity to be our child, and link us as their parent if specified.
        void addChild(GameEntity& child, bool updateChild = true);
        const std::vector<GameEntity*>& children() const;

        /// Forget a child of ours.
        void forgetChild(GameEntity& child, bool updateChild = true);

        /// Index of the child within the children list. Returns -1u if not in list.
        uint32_t childIndex(const GameEntity& child) const;
        /// @}

        /**
         * @name Components
         */
        /// @{
        /// The list of all components.
        const std::vector<std::string>& componentsHrids() const { return m_componentsHrids; }

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
        /// @}

        /// Tools
        /// Returns 0.f if no intersection. Never returns a negative.
        float distanceFrom(Ray ray, PickPrecision pickPrecision = PickPrecision::Mesh) const;

    public:
        class Impl;
        Impl& impl() { return *m_impl; }
        const Impl& impl() const { return *m_impl; }

    private:
        GameEngine& m_engine;
        Impl* m_impl = nullptr;

        std::vector<std::string> m_componentsHrids;
    };
}

#include <lava/sill/game-entity.inl>
