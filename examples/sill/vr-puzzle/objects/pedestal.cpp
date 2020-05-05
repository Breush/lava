#include "./pedestal.hpp"

#include "../game-state.hpp"

using namespace lava;

Pedestal::Pedestal(GameState& gameState)
    : Generic(gameState)
{
    m_bricksRoot = &gameState.engine->make<sill::GameEntity>("pedestal.bricks-root");
    m_bricksRoot->make<sill::AnimationComponent>();
    m_bricksRoot->ensure<sill::TransformComponent>().scaling(0.f);
    m_bricksRoot->get<sill::TransformComponent>().translation({0.f, 0.f, 0.75f});
}

void Pedestal::clear(bool removeFromLevel)
{
    Generic::clear(removeFromLevel);

    m_gameState.engine->remove(*m_bricksRoot);
    m_brickInfos.clear();
}

void Pedestal::unserialize(const nlohmann::json& data)
{
    m_material = data["material"].get<std::string>();

    m_brickInfos.clear();
    for (auto& brick : data["bricks"]) {
        m_brickInfos.emplace_back(BrickInfo{.unconsolidatedBrickId = brick});
    }
}

nlohmann::json Pedestal::serialize() const
{
    nlohmann::json data;
    data["material"] = m_material;

    data["bricks"] = nlohmann::json::array();
    for (auto& brickInfo : m_brickInfos) {
        data["bricks"].emplace_back(findBrickIndex(m_gameState, brickInfo.brick->entity()));
    }

    return data;
}

void Pedestal::consolidateReferences()
{
    m_entity->addChild(*m_bricksRoot);

    for (auto& brickInfo : m_brickInfos) {
        brickInfo.brick = m_gameState.level.bricks[brickInfo.unconsolidatedBrickId].get();
        brickInfo.brick->stored(true); // @fixme Have stored serialized, and we won't need that here.
        m_bricksRoot->addChild(brickInfo.brick->entity());
    }

    m_entity->ensure<sill::BehaviorComponent>().onUpdate([this](float dt) {
        if (!m_powered) return;

        for (auto& brickInfo : m_brickInfos) {
            auto& brick = *brickInfo.brick;
            if (!brick.stored()) continue;

            brickInfo.rotation1 += dt;
            brickInfo.rotation2 += 0.25f * dt;

            lava::Transform target;
            target.rotation = glm::rotate(target.rotation, brickInfo.rotation1, {0.f, 2.f, 1.f});
            target.rotation = glm::rotate(target.rotation, brickInfo.rotation2, {1.f, 0.f, 2.f});

            brick.animation().target(sill::AnimationFlag::WorldTransform, m_bricksRoot->get<sill::TransformComponent>().worldTransform() * target);
        }
    });

    // @fixme Left to implement:
    // - Close animation (open reversed?)
    // - Multiple bricks animations
}

void Pedestal::powered(bool powered)
{
    if (m_powered == powered) return;
    m_powered = powered;

    if (m_powered) {
        mesh().startAnimation("open");
    }

    lava::Transform bricksRootTarget;
    bricksRootTarget.translation = glm::vec3{0.f, 0.f, (m_powered) ? 1.25f : 0.75f};
    bricksRootTarget.scaling = (m_powered) ? 1.f : 0.f;

    auto& bricksRootAnimation = m_bricksRoot->get<sill::AnimationComponent>();
    bricksRootAnimation.start(sill::AnimationFlag::Transform, 1.f);
    bricksRootAnimation.target(sill::AnimationFlag::Transform, bricksRootTarget);
}
