#include "./barrier.hpp"

#include "../game-state.hpp"

using namespace lava;

Barrier::Barrier(GameState& gameState)
    : Object(gameState)
{
    gameState.level.barriers.emplace_back(this);

    m_entity->name("barrier");
    m_entity->make<sill::TransformComponent>();
    m_entity->make<sill::MeshComponent>();
    m_entity->make<sill::AnimationComponent>();

    auto& meshComponent = m_entity->get<sill::MeshComponent>();
    sill::makers::CylinderMeshOptions options = {.doubleSided = true, .offset = 0.125f};
    sill::makers::cylinderMeshMaker(16u, 1.f, 0.25f, options)(meshComponent);
    meshComponent.primitive(0u, 0u).category(RenderCategory::Translucent);
    meshComponent.primitive(0u, 0u).shadowsCastable(false);

    auto barrierMaterial = gameState.engine->scene().makeMaterial("barrier");
    meshComponent.primitive(0u, 0u).material(barrierMaterial);
}

void Barrier::clear(bool removeFromLevel)
{
    if (removeFromLevel) {
        for (auto brick : m_gameState.level.bricks) {
            brick->removeBarrier(*this);
        }

        for (auto panel : m_gameState.level.panels) {
            panel->removeBarrier(*this);
        }

        auto barrierIt = std::find(m_gameState.level.barriers.begin(), m_gameState.level.barriers.end(), this);
        m_gameState.level.barriers.erase(barrierIt);
    }

    // @note Keep last, this destroys us!
    Object::clear(removeFromLevel);
}

void Barrier::unserialize(const nlohmann::json& data)
{
    diameter(data["diameter"]);
    powered(data["powered"]);
}

nlohmann::json Barrier::serialize() const
{
    nlohmann::json data = {
        {"diameter", m_diameter},
        {"powered", m_powered},
    };
    return data;
}

// -----

void Barrier::diameter(float diameter)
{
    if (m_diameter == diameter) return;
    m_diameter = diameter;

    auto matrix = glm::scale(glm::mat4(1.f), {diameter, diameter, 1.f});
    mesh().node(0u).transform(matrix); // @todo We went through this setter, maybe we don't really need the explicit call below.
    mesh().dirtifyNodesTransforms();
}

void Barrier::powered(bool powered)
{
    if (m_powered == powered) return;
    m_powered = powered;

    animation().start(sill::AnimationFlag::MaterialUniform, material(), "poweredRatio", 0.5f);
    animation().target(sill::AnimationFlag::MaterialUniform, material(), "poweredRatio", (powered) ? 1.f : 0.f);
}

// -----

Barrier* findBarrierByName(GameState& gameState, const std::string& name)
{
    for (auto barrier : gameState.level.barriers) {
        if (barrier->name() == name) {
            return barrier;
        }
    }

    return nullptr;
}

Barrier* findBarrier(GameState& gameState, const sill::GameEntity& entity)
{
    for (auto barrier : gameState.level.barriers) {
        if (&barrier->entity() == &entity) {
            return barrier;
        }
    }

    return nullptr;
}

uint32_t findBarrierIndex(GameState& gameState, const sill::GameEntity& entity)
{
    for (uint32_t i = 0u; i < gameState.level.barriers.size(); ++i) {
        auto barrier = gameState.level.barriers[i];
        if (&barrier->entity() == &entity) {
            return i;
        }
    }

    return -1u;
}
