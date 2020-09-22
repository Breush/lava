#pragma once

namespace lava::sill {
    template <class IComponentClass>
    class ComponentHolder {
    public:
        ComponentHolder(bool pendingAddingEnabled = true)
            : m_pendingAddingEnabled(pendingAddingEnabled)
        {
        }

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
        IComponentClass& getComponent(const std::string& hrid);
        template <class ComponentClass>
        const ComponentClass& get() const;
        const IComponentClass& getComponent(const std::string& hrid) const;

        /// Add a created component to this holder. The holder handles its lifetime.
        void add(const std::string& hrid, std::unique_ptr<IComponentClass>&& component);

        /// Remove a previously added (or made) component.
        template <class ComponentClass>
        void remove();
        void removeComponent(const std::string& hrid);
        /// @}

    protected:
        void addPendingComponents();

    protected:
        std::vector<std::string> m_componentsHrids;
        std::unordered_map<std::string, std::unique_ptr<IComponentClass>> m_components;
        std::unordered_map<std::string, std::unique_ptr<IComponentClass>> m_pendingAddedComponents;

    private:
        bool m_pendingAddingEnabled = true;
    };
}

#include <lava/sill/component-holder.inl>
