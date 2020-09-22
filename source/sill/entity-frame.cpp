#include <lava/sill/entity-frame.hpp>

#include <lava/sill/game-engine.hpp>
#include <lava/sill/entity.hpp>

using namespace lava::sill;
using namespace lava::chamber;

EntityFrame::EntityFrame(GameEngine& engine)
    : ComponentHolder(false)
    , m_engine(engine)
{
}

// ----- Instancing

Entity& EntityFrame::makeEntity()
{
    auto& entity = m_engine.make<Entity>();
    return makeEntity(entity);
}

Entity& EntityFrame::makeEntity(Entity& entity)
{
    m_entities.emplace_back(&entity);

    entity.m_frame = this;

    for (auto& component : m_components) {
        component.second->makeEntity(entity);
    }

    return entity;
}

// ----- Internal

void EntityFrame::warnRemoved()
{
    if (!m_entities.empty()) {
        logger.warning("sill.entity-frame") << "Entity frame still has " << m_entities.size() << " entities alive upon removal." << std::endl;
    }
}

void EntityFrame::warnEntityRemoved(Entity& entity)
{
    auto entityIt = std::find(m_entities.begin(), m_entities.end(), &entity);
    m_entities.erase(entityIt);
    entity.m_frame = nullptr;

    for (auto& component : m_components) {
        component.second->warnEntityRemoved(entity);
    }
}
