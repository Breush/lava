#include "./game-entity-impl.hpp"

using namespace lava::chamber;
using namespace lava::sill;

GameEntity::Impl::Impl(GameEntity& entity, GameEngine& engine)
    : m_entity(entity)
    , m_engine(engine.impl())
{
}

GameEntity::Impl::~Impl()
{
    // Remove from parent if any
    if (m_parent) {
        m_parent->forgetChild(m_entity, false);
        m_parent = nullptr;
    }

    // Forget all children
    for (auto child : m_children) {
        child->parent(nullptr, false);
    }
}

void GameEntity::Impl::update(float dt)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

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

// ----- GameEntity hierarchy

void GameEntity::Impl::parent(GameEntity* parent, bool updateParent)
{
    if (updateParent && m_parent) {
        m_parent->forgetChild(m_entity, false);
    }

    m_parent = parent;

    if (updateParent && m_parent) {
        m_parent->addChild(m_entity, false);
    }
}

void GameEntity::Impl::addChild(GameEntity& child, bool updateChild)
{
    m_children.emplace_back(&child);

    if (updateChild) {
        child.parent(m_entity, false);
    }
}

void GameEntity::Impl::forgetChild(GameEntity& child, bool updateChild)
{
    auto iChild = std::find(m_children.begin(), m_children.end(), &child);
    if (iChild != m_children.end()) {
        m_children.erase(iChild);
    }

    if (updateChild) {
        child.parent(nullptr, false);
    }
}

// ----- GameEntity components

bool GameEntity::Impl::hasComponent(const std::string& hrid) const
{
    return m_pendingAddedComponents.find(hrid) != m_pendingAddedComponents.end() || m_components.find(hrid) != m_components.end();
}

IComponent& GameEntity::Impl::getComponent(const std::string& hrid)
{
    auto pComponent = m_pendingAddedComponents.find(hrid);
    if (pComponent != m_pendingAddedComponents.end()) return *pComponent->second;
    if (!hasComponent(hrid)) {
        logger.error("sill.game-entity") << "Entity '" << m_name << "' has no " << hrid << " component." << std::endl;
    }
    return *m_components.at(hrid);
}

const IComponent& GameEntity::Impl::getComponent(const std::string& hrid) const
{
    auto pComponent = m_pendingAddedComponents.find(hrid);
    if (pComponent != m_pendingAddedComponents.end()) return *pComponent->second;
    if (!hasComponent(hrid)) {
        logger.error("sill.game-entity") << "Entity '" << m_name << "' has no " << hrid << " component." << std::endl;
    }
    return *m_components.at(hrid);
}

void GameEntity::Impl::add(const std::string& hrid, std::unique_ptr<IComponent>&& component)
{
    if (hasComponent(hrid)) {
        logger.error("sill.game-entity") << "Entity '" << m_name << "' already has a " << hrid << " component." << std::endl;
    }

    logger.info("sill.game-entity").tab(2) << "(" << this << " '" << m_name << "') Adding " << hrid << " component." << std::endl;

    m_pendingAddedComponents.emplace(hrid, std::move(component));

    logger.log().tab(-2);
}

void GameEntity::Impl::removeComponent(const std::string& hrid)
{
    m_components.erase(m_components.find(hrid));
}
