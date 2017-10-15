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
    for (auto& component : m_components) {
        component.second->update();
    }
}

void GameEntity::Impl::postUpdate()
{
    for (auto& component : m_components) {
        component.second->postUpdate();
    }
}

IComponent& GameEntity::Impl::getComponent(const std::string& hrid)
{
    return *m_components.at(hrid);
}

const IComponent& GameEntity::Impl::getComponent(const std::string& hrid) const
{
    return *m_components.at(hrid);
}

void GameEntity::Impl::add(const std::string& hrid, std::unique_ptr<IComponent>&& component)
{
    m_components.emplace(hrid, std::move(component));
}

void GameEntity::Impl::removeComponent(const std::string& hrid)
{
    m_components.erase(m_components.find(hrid));
}
