#include <lava/sill/component-holder.hpp>

#include <lava/sill/components/i-component.hpp>

using namespace lava::sill;
using namespace lava::chamber;

void ComponentHolder::componentsUpdate(float dt)
{
    // Add all new components
    for (auto& component : m_pendingAddedComponents) {
        m_components.emplace(std::move(component));
    }
    m_pendingAddedComponents.clear();

    // Effective update
    for (auto& component : m_components) {
        component.second->update(dt);
    }

    // @todo Remove components asynchronously too
}

void ComponentHolder::componentsUpdateFrame()
{
    // Effective update
    for (auto& component : m_components) {
        component.second->updateFrame();
    }
}

bool ComponentHolder::hasComponent(const std::string& hrid) const
{
    return m_pendingAddedComponents.find(hrid) != m_pendingAddedComponents.end() || m_components.find(hrid) != m_components.end();
}

IComponent& ComponentHolder::getComponent(const std::string& hrid)
{
    auto pComponent = m_pendingAddedComponents.find(hrid);
    if (pComponent != m_pendingAddedComponents.end()) return *pComponent->second;
    if (!hasComponent(hrid)) {
        logger.error("sill.component-holder") << "No " << hrid << " component." << std::endl;
    }
    return *m_components.at(hrid);
}

const IComponent& ComponentHolder::getComponent(const std::string& hrid) const
{
    auto pComponent = m_pendingAddedComponents.find(hrid);
    if (pComponent != m_pendingAddedComponents.end()) return *pComponent->second;
    if (!hasComponent(hrid)) {
        logger.error("sill.component-holder") << "No " << hrid << " component." << std::endl;
    }
    return *m_components.at(hrid);
}

void ComponentHolder::add(const std::string& hrid, std::unique_ptr<IComponent>&& component)
{
    if (hasComponent(hrid)) {
        logger.error("sill.component-holder") << "Already has a " << hrid << " component." << std::endl;
    }

    logger.info("sill.component-holder").tab(2) << "(" << this << "') Adding " << hrid << " component." << std::endl;

    m_pendingAddedComponents.emplace(hrid, std::move(component));

    logger.log().tab(-2);
}

void ComponentHolder::removeComponent(const std::string& hrid)
{
    m_components.erase(m_components.find(hrid));
}
