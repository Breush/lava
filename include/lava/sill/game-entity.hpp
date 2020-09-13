#pragma once

#include <lava/sill/component-holder.hpp>
#include <lava/sill/pick-precision.hpp>

#include <lava/core/ray.hpp>
#include <memory>

namespace lava::sill {
    class GameEngine;
}

namespace lava::sill {
    /**
     * A simple game object that can have components.
     */
    // @fixme Rename GameEntity -> Entity (namespace is sufficient)
    class GameEntity final : public ComponentHolder {
    public:
        using ParentChangedCallback = std::function<void()>;

    public:
        GameEntity(GameEngine& engine);
        GameEntity(GameEngine& engine, const std::string& name);
        GameEntity(const GameEntity& entity) = delete;
        ~GameEntity();

        void update(float dt);
        void updateFrame();

        GameEngine& engine() { return m_engine; }
        const GameEngine& engine() const { return m_engine; }

        /**
         * @name Attributes
         */
        /// @{
        const std::string& name() const { return m_name; }
        void name(const std::string& name) { m_name = name; }

        /// An inactive entity will never update.
        bool active() const { return m_active && m_alive; }
        void active(bool active) { m_active = active; }

        /// Called by GameEngine when an entity will be removed.
        void alive(bool alive) { m_alive = alive; }
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
        GameEntity* parent() { return m_parent; }
        const GameEntity* parent() const { return m_parent; }
        void parent(GameEntity& parent, bool updateParent = true) { this->parent(&parent, updateParent); }
        void parent(GameEntity* parent, bool updateParent = true);

        /// Sets an entity to be our child, and link us as their parent if specified.
        void addChild(GameEntity& child, bool updateChild = true);
        const std::vector<GameEntity*>& children() const { return m_children; }

        /// Forget a child of ours.
        void forgetChild(GameEntity& child, bool updateChild = true);

        /// Index of the child within the children list. Returns -1u if not in list.
        uint32_t childIndex(const GameEntity& child) const;
        /// @}

        /**
         * @name Components
         */
        /// @{
        /// Create a new component on this entity. The entity handles its lifetime.
        template <class ComponentClass, class... Arguments>
        ComponentClass& make(Arguments&&... arguments);

        /// Ensure this entity has the required component. If it does not, make it.
        template <class ComponentClass>
        ComponentClass& ensure();
        /// @}

        /**
         * @name Callbacks
         */
        /// @{
        /// Be warned whenever the parent of the entity changes.
        void onParentChanged(ParentChangedCallback callback) { m_parentChangedCallbacks.emplace_back(callback); }
        /// @}

        /// Tools
        /// Returns 0.f if no intersection. Never returns a negative.
        float distanceFrom(const Ray& ray, PickPrecision pickPrecision = PickPrecision::Mesh) const;

    private:
        // References
        GameEngine& m_engine;

        // Attributes
        std::string m_name = "<unknown>";
        bool m_alive = true;
        bool m_active = true;

        // Hierarchy
        GameEntity* m_parent = nullptr; // Keep nullptr to be top-level.
        std::vector<GameEntity*> m_children;
        std::vector<ParentChangedCallback> m_parentChangedCallbacks;
    };
}

#include <lava/sill/game-entity.inl>
