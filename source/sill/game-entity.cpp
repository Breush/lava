#include <lava/sill/game-entity.hpp>

#include "./game-entity-impl.hpp"

using namespace lava::sill;

GameEntity::GameEntity(GameEngine& engine)
    : m_engine(engine)
{
    m_impl = new GameEntity::Impl(engine);
}
GameEntity::~GameEntity()
{
    delete m_impl;
}

$pimpl_method_const(GameEntity, bool, hasComponent, const std::string&, hrid);
$pimpl_method(GameEntity, IComponent&, getComponent, const std::string&, hrid);
$pimpl_method_const(GameEntity, const IComponent&, getComponent, const std::string&, hrid);

void GameEntity::add(const std::string& hrid, std::unique_ptr<IComponent>&& component)
{
    m_impl->add(hrid, std::move(component));
}

$pimpl_method(GameEntity, void, removeComponent, const std::string&, hrid);
