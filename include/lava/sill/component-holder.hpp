#pragma once

namespace lava::sill {
    class IComponent;
}

namespace lava::sill {
    class ComponentHolder {
    public:
        // Forwarded to components.
        void componentsUpdate(float dt);
        void componentsUpdateFrame();

        /**
         * @name Components
         */
        /// @{
        /// The list of all components.
        const std::vector<std::string>& componentsHrids() const { return m_componentsHrids; }

        /// Check if the specified component exists within the holder.
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

        /// Add a created component to this holder. The holder handles its lifetime.
        void add(const std::string& hrid, std::unique_ptr<IComponent>&& component);

        /// Remove a previously added (or made) component.
        template <class ComponentClass>
        void remove();
        void removeComponent(const std::string& hrid);
        /// @}

    protected:
        std::vector<std::string> m_componentsHrids;
        std::unordered_map<std::string, std::unique_ptr<IComponent>> m_components;
        std::unordered_map<std::string, std::unique_ptr<IComponent>> m_pendingAddedComponents;
    };
}

#include <lava/sill/component-holder.inl>
