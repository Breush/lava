#include <lava/sill/entity.hpp>

#include <lava/sill/game-engine.hpp>
#include <lava/sill/entity-frame.hpp>
#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/components/physics-component.hpp>

using namespace lava::sill;

namespace {
    void callParentChanged(const std::vector<Entity::ParentChangedCallback>& callbacks)
    {
        for (const auto& callback : callbacks) {
            callback();
        }
    }
}

Entity::Entity(GameEngine& engine)
    : m_engine(engine)
{
}

Entity::Entity(GameEngine& engine, const std::string& name)
    : Entity(engine)
{
    m_name = name;
}

Entity::~Entity()
{
    if (m_engine.destroying()) return;

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

void Entity::update(float dt)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // Add all new components
    addPendingComponents();

    for (auto& component : m_components) {
        component.second->update(dt);
    }
}

void Entity::updateFrame()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    for (auto& component : m_components) {
        component.second->updateFrame();
    }
}

// ----- Hierarchy

void Entity::parent(Entity* parent, bool updateParent)
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

void Entity::addChild(Entity& child, bool updateChild)
{
    m_children.emplace_back(&child);

    if (updateChild) {
        child.parent(*this, false);
    }
}

void Entity::forgetChild(Entity& child, bool updateChild)
{
    auto iChild = std::find(m_children.begin(), m_children.end(), &child);
    if (iChild != m_children.end()) {
        m_children.erase(iChild);
    }

    if (updateChild) {
        child.parent(nullptr, false);
    }
}

uint32_t Entity::childIndex(const Entity& child) const
{
    const auto& childrenList = children();
    for (auto i = 0u; i < childrenList.size(); ++i) {
        if (childrenList[i] == &child) return i;
    }

    return -1u;
}

// ----- Tools

float Entity::distanceFrom(const Ray& ray, PickPrecision pickPrecision) const
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

// ----- Internal

void Entity::warnRemoved()
{
    m_alive = false;

    if (m_frame != nullptr) {
        m_frame->warnEntityRemoved(*this);
    }
}
