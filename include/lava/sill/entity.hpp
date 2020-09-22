#pragma once

#include <lava/sill/component-holder.hpp>
#include <lava/sill/pick-precision.hpp>
#include <lava/sill/components/i-component.hpp>

#include <lava/core/ray.hpp>
#include <memory>

namespace lava::sill {
    class GameEngine;
    class EntityFrame;
}

namespace lava::sill {
    /**
     * A simple game object that can have components.
     */
    class Entity final : public ComponentHolder<IComponent> {
    public:
        friend class GameEngine;
        friend class EntityFrame;
        using ParentChangedCallback = std::function<void()>;

    public:
        Entity(GameEngine& engine);
        Entity(GameEngine& engine, const std::string& name);
        Entity(const Entity& entity) = delete;
        ~Entity();

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

        /// The frame it has been constructed from if any.
        EntityFrame* frame() const { return m_frame; }
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
        Entity* parent() { return m_parent; }
        const Entity* parent() const { return m_parent; }
        void parent(Entity& parent, bool updateParent = true) { this->parent(&parent, updateParent); }
        void parent(Entity* parent, bool updateParent = true);

        /// Sets an entity to be our child, and link us as their parent if specified.
        void addChild(Entity& child, bool updateChild = true);
        const std::vector<Entity*>& children() const { return m_children; }

        /// Forget a child of ours.
        void forgetChild(Entity& child, bool updateChild = true);

        /// Index of the child within the children list. Returns -1u if not in list.
        uint32_t childIndex(const Entity& child) const;
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

    protected:
        void warnRemoved();

    private:
        // References
        GameEngine& m_engine;
        EntityFrame* m_frame = nullptr;

        // Attributes
        std::string m_name = "<unknown>";
        bool m_alive = true;
        bool m_active = true;

        // Hierarchy
        Entity* m_parent = nullptr; // Keep nullptr to be top-level.
        std::vector<Entity*> m_children;
        std::vector<ParentChangedCallback> m_parentChangedCallbacks;
    };
}

#include <lava/sill/entity.inl>
