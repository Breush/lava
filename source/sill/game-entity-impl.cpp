#include "./game-entity-impl.hpp"

#include <lava/chamber/logger.hpp>

using namespace lava::chamber;
using namespace lava::sill;

GameEntity::Impl::Impl(GameEngine& engine)
    : m_engine(engine.impl())
{
}

void GameEntity::Impl::update()
{
    // Add all new components
    for (auto& component : m_pendingAddedComponents) {
        m_components.emplace(std::move(component));
        // m_components.emplace(component.first, std::move(component.second));
    }
    m_pendingAddedComponents.clear();

    // Effective update
    for (auto& component : m_components) {
        component.second->update();
    }

    // @todo Remove components asynchronously too
}

bool GameEntity::Impl::hasComponent(const std::string& hrid) const
{
    return m_pendingAddedComponents.find(hrid) != m_pendingAddedComponents.end() || m_components.find(hrid) != m_components.end();
}

IComponent& GameEntity::Impl::getComponent(const std::string& hrid)
{
    auto pComponent = m_pendingAddedComponents.find(hrid);
    if (pComponent != m_pendingAddedComponents.end()) return *pComponent->second;
    return *m_components.at(hrid);
}

const IComponent& GameEntity::Impl::getComponent(const std::string& hrid) const
{
    auto pComponent = m_pendingAddedComponents.find(hrid);
    if (pComponent != m_pendingAddedComponents.end()) return *pComponent->second;
    return *m_components.at(hrid);
}

void GameEntity::Impl::add(const std::string& hrid, std::unique_ptr<IComponent>&& component)
{
    if (hasComponent(hrid)) {
        logger.error("sill.game-entity") << "This entity already has a " << hrid << " component." << std::endl;
    }

    logger.info("sill.game-entity") << "(" << this << ") Adding " << hrid << " component." << std::endl;

    m_pendingAddedComponents.emplace(hrid, std::move(component));
}

void GameEntity::Impl::removeComponent(const std::string& hrid)
{
    m_components.erase(m_components.find(hrid));
}
