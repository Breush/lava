#pragma once

#include <lava/chamber/logger.hpp>

namespace lava::sill {
    template <class IComponentClass>
    template <class ComponentClass>
    bool ComponentHolder<IComponentClass>::has() const
    {
        return hasComponent(ComponentClass::hrid());
    }

    template <class IComponentClass>
    bool ComponentHolder<IComponentClass>::hasComponent(const std::string& hrid) const
    {
        return (m_pendingAddedComponents.find(hrid) != m_pendingAddedComponents.end()) ||
               (m_components.find(hrid) != m_components.end());
    }

    template <class IComponentClass>
    template <class ComponentClass>
    ComponentClass& ComponentHolder<IComponentClass>::get()
    {
        auto& component = getComponent(ComponentClass::hrid());
        return reinterpret_cast<ComponentClass&>(component);
    }

    template <class IComponentClass>
    IComponentClass& ComponentHolder<IComponentClass>::getComponent(const std::string& hrid)
    {
        auto pComponent = m_pendingAddedComponents.find(hrid);
        if (pComponent != m_pendingAddedComponents.end()) return *pComponent->second;
        if (!hasComponent(hrid)) {
            lava::chamber::logger.error("sill.component-holder") << "No " << hrid << " component." << std::endl;
        }
        return *m_components.at(hrid);
    }

    template <class IComponentClass>
    template <class ComponentClass>
    const ComponentClass& ComponentHolder<IComponentClass>::get() const
    {
        const auto& component = getComponent(ComponentClass::hrid());
        return reinterpret_cast<const ComponentClass&>(component);
    }

    template <class IComponentClass>
    const IComponentClass& ComponentHolder<IComponentClass>::getComponent(const std::string& hrid) const
    {
        auto pComponent = m_pendingAddedComponents.find(hrid);
        if (pComponent != m_pendingAddedComponents.end()) return *pComponent->second;
        if (!hasComponent(hrid)) {
            lava::chamber::logger.error("sill.component-holder") << "No " << hrid << " component." << std::endl;
        }
        return *m_components.at(hrid);
    }

    template <class IComponentClass>
    void ComponentHolder<IComponentClass>::add(const std::string& hrid, std::unique_ptr<IComponentClass>&& component)
    {
        if (hasComponent(hrid)) {
            lava::chamber::logger.error("sill.component-holder") << "Already has a " << hrid << " component." << std::endl;
        }

        lava::chamber::logger.info("sill.component-holder").tab(2) << "(" << this << "') Adding " << hrid << " component." << std::endl;

        m_componentsHrids.emplace_back(hrid);
        if (m_pendingAddingEnabled) {
            m_pendingAddedComponents.emplace(hrid, std::move(component));
        } else {
            m_components.emplace(hrid, std::move(component));
        }

        lava::chamber::logger.log().tab(-2);
    }

    template <class IComponentClass>
    template <class ComponentClass>
    void ComponentHolder<IComponentClass>::remove()
    {
        removeComponent(ComponentClass::hrid());
    }

    template <class IComponentClass>
    void ComponentHolder<IComponentClass>::removeComponent(const std::string& hrid)
    {
        m_components.erase(m_components.find(hrid));
    }

    template <class IComponentClass>
    void ComponentHolder<IComponentClass>::addPendingComponents()
    {
        for (auto& component : m_pendingAddedComponents) {
            m_components.emplace(std::move(component));
        }
        m_pendingAddedComponents.clear();
    }
}
