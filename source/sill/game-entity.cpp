#include <lava/sill/game-entity.hpp>

#include <lava/sill/components/mesh-component.hpp>

#include "./game-entity-impl.hpp"

using namespace lava::sill;

GameEntity::GameEntity(GameEngine& engine)
    : m_engine(engine)
{
    m_impl = new GameEntity::Impl(*this, engine);
}

GameEntity::GameEntity(GameEngine& engine, const std::string& name)
    : GameEntity(engine)
{
    this->name(name);
}

GameEntity::~GameEntity()
{
    delete m_impl;
}

// Hierarchy
$pimpl_method(GameEntity, GameEntity*, parent);
$pimpl_method_const(GameEntity, const GameEntity*, parent);
$pimpl_method(GameEntity, void, parent, GameEntity&, parent);
$pimpl_method(GameEntity, void, parent, GameEntity*, parent);
$pimpl_method(GameEntity, void, addChild, GameEntity&, child);
$pimpl_method_const(GameEntity, const std::vector<GameEntity*>&, children);

uint32_t GameEntity::childIndex(const GameEntity& child) const
{
    const auto& childrenList = children();
    for (auto i = 0u; i < childrenList.size(); ++i) {
        if (childrenList[i] == &child) return i;
    }

    return -1u;
}

// Components
$pimpl_method_const(GameEntity, bool, hasComponent, const std::string&, hrid);
$pimpl_method(GameEntity, IComponent&, getComponent, const std::string&, hrid);
$pimpl_method_const(GameEntity, const IComponent&, getComponent, const std::string&, hrid);

void GameEntity::add(const std::string& hrid, std::unique_ptr<IComponent>&& component)
{
    m_impl->add(hrid, std::move(component));
    m_componentsHrids.emplace_back(hrid);
}

$pimpl_method(GameEntity, void, removeComponent, const std::string&, hrid);

float GameEntity::distanceFrom(Ray ray, PickPrecision pickPrecision) const
{
    if (!has<MeshComponent>()) return 0.f;

    const auto& meshComponent = get<MeshComponent>();
    return meshComponent.distanceFrom(ray, pickPrecision);
}
