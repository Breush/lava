#include <lava/sill/game-entity.hpp>

#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/components/physics-component.hpp>

using namespace lava::sill;

namespace {
    void callParentChanged(const std::vector<GameEntity::ParentChangedCallback>& callbacks)
    {
        for (const auto& callback : callbacks) {
            callback();
        }
    }
}

GameEntity::GameEntity(GameEngine& engine)
    : m_engine(engine)
{
}

GameEntity::GameEntity(GameEngine& engine, const std::string& name)
    : GameEntity(engine)
{
    m_name = name;
}

GameEntity::~GameEntity()
{
    // Remove from parent if any
    if (m_parent) {
        m_parent->forgetChild(*this, false);
        m_parent = nullptr;
    }

    // Forget all children
    for (auto child : m_children) {
        child->parent(nullptr, false);
    }
}

void GameEntity::update(float dt)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    componentsUpdate(dt);
}

void GameEntity::updateFrame()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    componentsUpdateFrame();
}

// ----- Hierarchy

void GameEntity::parent(GameEntity* parent, bool updateParent)
{
    if (m_parent == parent) return;

    if (updateParent && m_parent) {
        m_parent->forgetChild(*this, false);
    }

    m_parent = parent;

    if (updateParent && m_parent) {
        m_parent->addChild(*this, false);
    }

    callParentChanged(m_parentChangedCallbacks);
}

void GameEntity::addChild(GameEntity& child, bool updateChild)
{
    m_children.emplace_back(&child);

    if (updateChild) {
        child.parent(*this, false);
    }
}

void GameEntity::forgetChild(GameEntity& child, bool updateChild)
{
    auto iChild = std::find(m_children.begin(), m_children.end(), &child);
    if (iChild != m_children.end()) {
        m_children.erase(iChild);
    }

    if (updateChild) {
        child.parent(nullptr, false);
    }
}

uint32_t GameEntity::childIndex(const GameEntity& child) const
{
    const auto& childrenList = children();
    for (auto i = 0u; i < childrenList.size(); ++i) {
        if (childrenList[i] == &child) return i;
    }

    return -1u;
}

// ----- Tools

float GameEntity::distanceFrom(const Ray& ray, PickPrecision pickPrecision) const
{
    if (pickPrecision == PickPrecision::Collider) {
        if (!has<PhysicsComponent>()) return 0.f;
        const auto& physicsComponent = get<PhysicsComponent>();
        return physicsComponent.distanceFrom(ray);
    }

    if (!has<MeshComponent>()) return 0.f;

    const auto& meshComponent = get<MeshComponent>();
    return meshComponent.distanceFrom(ray, pickPrecision);
}
