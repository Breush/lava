#include "./pedestal.hpp"

#include "../game-state.hpp"

using namespace lava;

Pedestal::Pedestal(GameState& gameState)
    : Object(gameState)
{
    m_bricksRoot = &gameState.engine->make<sill::Entity>("pedestal.bricks-root");
    m_bricksRoot->make<sill::AnimationComponent>();
    m_bricksRoot->ensure<sill::TransformComponent>().scaling(0.f);
    m_bricksRoot->get<sill::TransformComponent>().translation({0.f, 0.f, 0.75f});

    m_entity->name("pedestal");
    m_entity->ensure<sill::TransformComponent>();
    m_entity->addChild(*m_bricksRoot);
}

void Pedestal::clear(bool removeFromLevel)
{
    if (removeFromLevel) {
        for (auto& brickInfo : m_brickInfos) {
            brickInfo.brick->pedestal(nullptr);
            brickInfo.brick->stored(false);
        }
    }

    m_gameState.engine->remove(*m_bricksRoot);

    for (auto& brickInfo : m_brickInfos) {
        m_gameState.engine->remove(*brickInfo.arm);
    }

    // @note Keep last, this destroys us!
    Object::clear(removeFromLevel);
}

void Pedestal::unserialize(const nlohmann::json& data)
{
    m_substanceRevealNeeded = data["substanceRevealNeeded"].get<std::string>();

    m_brickInfos.clear();
    for (auto& brick : data["bricks"]) {
        auto& brickInfo = m_brickInfos.emplace_back(BrickInfo{.unconsolidatedBrickId = brick});
        brickInfo.arm = &m_gameState.engine->make<sill::Entity>("pedestal.bricks-root.arm");
        brickInfo.arm->ensure<sill::TransformComponent>();
        brickInfo.arm->parent(m_bricksRoot);
    }
}

nlohmann::json Pedestal::serialize() const
{
    nlohmann::json data = {
        {"substanceRevealNeeded", m_substanceRevealNeeded},
        {"bricks", nlohmann::json::array()},
    };

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
        sill::makers::glbMeshMaker("./assets/models/vr-puzzle/" + m_substanceRevealNeeded + "-pedestal.glb")(meshComponent);
    }

    for (auto& brickInfo : m_brickInfos) {
        brickInfo.brick = m_gameState.level.bricks[brickInfo.unconsolidatedBrickId];
        brickInfo.brick->pedestal(this);
        brickStoredChanged(*brickInfo.brick);
    }

    updateBricksArms();

    m_entity->ensure<sill::BehaviorComponent>().onUpdate([this](float dt) {
        if (!m_powered) return;
        if (m_gameState.state == State::Editor) return;

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
}

void Pedestal::mutateBeforeDuplication(nlohmann::json& data)
{
    // @note The bricks we had attached to us cannot be attached
    // to two pedestal. Therefore, this information cannot be duplicated.
    data["bricks"] = nlohmann::json::array();
}

// ----- Editor controls

void Pedestal::uiWidgets(std::vector<UiWidget>& widgets)
{
    Object::uiWidgets(widgets);

    widgets.emplace_back(UiWidget("substanceRevealNeeded",
        [this]() -> const std::string& { return substanceRevealNeeded(); },
        [this](const std::string& substanceRevealNeeded) { this->substanceRevealNeeded(substanceRevealNeeded); })
    );
}

// -----

void Pedestal::brickStoredChanged(Brick& brick)
{
    auto brickInfoIt = std::find_if(m_brickInfos.begin(), m_brickInfos.end(), [&brick](const BrickInfo& brickInfo) {
        return brickInfo.brick == &brick;
    });

    auto stored = brick.stored();
    brick.entity().parent((stored) ? brickInfoIt->arm : nullptr);

    if (stored) {
        brick.animation().start(lava::sill::AnimationFlag::Transform, 1.f, false);
    } else {
        brick.animation().stop(lava::sill::AnimationFlag::Transform);
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
        brickInfo.arm = &m_gameState.engine->make<sill::Entity>("pedestal.bricks-root.arm");
        brickInfo.arm->ensure<sill::TransformComponent>();
        brickInfo.arm->parent(m_bricksRoot);
        m_brickInfos.emplace_back(brickInfo);
    }

    brick.transform().translation({0.f, 0.f, 0.f});
    brick.pedestal(this);
    brick.stored(true);

    updateBricksArms();
}

void Pedestal::powered(bool powered)
{
    if (m_powered == powered) return;
    m_powered = powered;

    // Closing is reversed open
    mesh().startAnimation("open", 1u, (m_powered) ? 1.f : -6.f);

    lava::Transform bricksRootTarget;
    bricksRootTarget.translation = glm::vec3{0.f, 0.f, (m_powered) ? 1.25f : 0.75f};
    bricksRootTarget.scaling = (m_powered) ? 1.f : 0.f;

    auto& bricksRootAnimation = m_bricksRoot->get<sill::AnimationComponent>();
    bricksRootAnimation.start(sill::AnimationFlag::Transform, (m_powered) ? 1.f : 0.3f);
    bricksRootAnimation.target(sill::AnimationFlag::Transform, bricksRootTarget);
}

// ----- Internal

void Pedestal::updateBricksArms()
{
    for (auto i = 0u; i < m_brickInfos.size(); ++i) {
        glm::vec3 translation{0.f, 0.f, i * 0.5f};
        m_brickInfos[i].rotation1 = i;
        m_brickInfos[i].rotation2 = i;
        m_brickInfos[i].arm->get<sill::TransformComponent>().translation(translation);
    }
}
