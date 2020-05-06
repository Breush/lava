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

    m_entity->name("pedestal");
    m_entity->ensure<sill::TransformComponent>();
    m_entity->addChild(*m_bricksRoot);
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
    // Create mesh if not yet referenced
    if (!m_entity->has<sill::MeshComponent>()) {
        auto& meshComponent = m_entity->make<sill::MeshComponent>();
        sill::makers::glbMeshMaker("./assets/models/vr-puzzle/" + m_material + "-pedestal.glb")(meshComponent);
    }

    for (auto& brickInfo : m_brickInfos) {
        brickInfo.brick = m_gameState.level.bricks[brickInfo.unconsolidatedBrickId].get();
        brickInfo.brick->pedestal(this);
        brickStoredChanged(*brickInfo.brick);
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

            brick.animation().target(sill::AnimationFlag::Transform, target);
        }
    });

    // @fixme Left to implement:
    // - Close animation (open reversed?)
    // - Multiple bricks animations
}

void Pedestal::brickStoredChanged(Brick& brick)
{
    auto stored = brick.stored();
    brick.entity().parent((stored) ? m_bricksRoot : nullptr);

    if (stored) {
        brick.animation().start(lava::sill::AnimationFlag::Transform, 1.f, false);
    }
}

void Pedestal::addBrick(Brick& brick)
{
    // Don't add the same brick twice!
    auto brickInfoIt = std::find_if(m_brickInfos.begin(), m_brickInfos.end(), [&brick](const BrickInfo& brickInfo) {
        return brickInfo.brick == &brick;
    });
    if (brickInfoIt == m_brickInfos.end()) {
        BrickInfo brickInfo;
        brickInfo.brick = &brick;
        m_brickInfos.emplace_back(brickInfo);
    }

    brick.transform().translation({0.f, 0.f, 0.f});
    brick.pedestal(this);
    brick.stored(true);
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
