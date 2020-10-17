#include "./panel.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "./brick.hpp"
#include "../environment.hpp"
#include "../game-state.hpp"
#include "../serializer.hpp"

using namespace lava;

namespace {
    constexpr const float thickness = 0.1f;
}

Panel::Panel(GameState& gameState)
    : Object(gameState)
{
    gameState.level.panels.emplace_back(this);

    auto& engine = *gameState.engine;

    // Create panel
    m_entity->name("panel");
    auto& meshComponent = m_entity->make<sill::MeshComponent>();
    sill::makers::PlaneMeshOptions options;
    options.rootNodeHasGeometry = false;
    auto rootNodeIndex = sill::makers::planeMeshMaker({blockExtent.x, blockExtent.y}, options)(meshComponent);
    m_material = engine.scene().makeMaterial("panel");
    meshComponent.primitive(1, 0).material(m_material);

    // Create border
    auto nodeIndex = meshComponent.addNode();
    meshComponent.nodeName(nodeIndex, "border");
    auto& group = meshComponent.nodeMakeGroup(nodeIndex);
    meshComponent.nodeAddAbsoluteChild(rootNodeIndex, nodeIndex);
    m_borderMaterial = engine.scene().makeMaterial("roughness-metallic");
    group.addPrimitive().material(m_borderMaterial);

    m_entity->ensure<sill::AnimationComponent>();
    m_entity->ensure<sill::TransformComponent>().onWorldTransformChanged([this] { updateSnappingPoints(); });
}

void Panel::clear(bool removeFromLevel)
{
    if (removeFromLevel) {
        for (auto brick : m_gameState.level.bricks) {
            if (&brick->snapPanel() == this) {
                brick->unsnap();
            }
        }

        auto panelIt = std::find(m_gameState.level.panels.begin(), m_gameState.level.panels.end(), this);
        m_gameState.level.panels.erase(panelIt);
    }

    // @note Keep last, this destroys us!
    Object::clear(removeFromLevel);
}

void Panel::unserialize(const nlohmann::json& data)
{
    m_extent = unserializeUvec2(data["extent"]);
    m_substanceRevealNeeded = data["substanceRevealNeeded"];
    m_substanceFreedOnSolve = data["substanceFreedOnSolve"];
    m_substanceRevealedOnSolve = data["substanceRevealedOnSolve"];
}

nlohmann::json Panel::serialize() const
{
    nlohmann::json data = {
        {"extent", ::serialize(m_extent)},
        {"substanceRevealNeeded", m_substanceRevealNeeded},
        {"substanceFreedOnSolve", m_substanceFreedOnSolve},
        {"substanceRevealedOnSolve", m_substanceRevealedOnSolve},
    };

    return data;
}

void Panel::consolidateReferences()
{
    m_consolidated = true;

    extent(m_extent);
}

void Panel::uiWidgets(std::vector<UiWidget>& widgets)
{
    Object::uiWidgets(widgets);

    widgets.emplace_back("pretendSolved", "pretendSolved",
        [this]() -> bool { return pretendSolved(); },
        [this](bool pretendSolved) { this->pretendSolved(pretendSolved); }
    );

    widgets.emplace_back(UiWidget("substanceRevealNeeded",
        [this]() -> const std::string& { return substanceRevealNeeded(); },
        [this](const std::string& substanceRevealNeeded) { this->substanceRevealNeeded(substanceRevealNeeded); })
    );

    widgets.emplace_back(UiWidget("substanceFreedOnSolve",
        [this]() -> const std::string& { return substanceFreedOnSolve(); },
        [this](const std::string& substanceFreedOnSolve) { this->substanceFreedOnSolve(substanceFreedOnSolve); })
    );

    widgets.emplace_back(UiWidget("substanceRevealedOnSolve",
        [this]() -> const std::string& { return substanceRevealedOnSolve(); },
        [this](const std::string& substanceRevealedOnSolve) { this->substanceRevealedOnSolve(substanceRevealedOnSolve); })
    );
}

// -----

bool Panel::userInteractionAllowed() const
{
    return m_powered;
}

void Panel::extent(const glm::uvec2& extent)
{
    m_extent = extent;
    m_material->set("extent", extent);
    m_entity->get<sill::MeshComponent>().nodeMatrix(1, glm::scale(glm::mat4(1.f), {m_extent.x, m_extent.y, 1}));

    updateBorderMeshPrimitive();

    updateSnappingPoints();
    updateSolved();
}

Panel::SnappingPoint* Panel::closestSnappingPoint(const Brick& brick, const glm::vec3& position, float minDistance)
{
    updateFromSnappedBricks();

    SnappingPoint* closestSnappingPoint = nullptr;

    for (auto& snappingPoints : m_snappingPoints) {
        for (auto& snappingPoint : snappingPoints) {
            auto distance = glm::distance(snappingPoint.worldTransform.translation, position);
            if (distance < minDistance) {
                if (!isSnappingPointValid(brick, snappingPoint)) continue;
                minDistance = distance;
                closestSnappingPoint = &snappingPoint;
            }
        }
    }

    return closestSnappingPoint;
}

Panel::SnappingInfo Panel::rayHitSnappingPoint(const Brick& brick, const lava::Ray& ray)
{
    const auto bsRadius = std::sqrt(blockExtent.x * blockExtent.x + blockExtent.y * blockExtent.y) / 2.f;

    SnappingInfo snappingInfo;
    snappingInfo.point = nullptr;

    float minDistance = 1000.f;
    for (auto& snappingPoints : m_snappingPoints) {
        for (auto& snappingPoint : snappingPoints) {
            auto bsCenter = snappingPoint.worldTransform.translation;
            auto rayOriginToBsCenter = bsCenter - ray.origin;
            auto bsCenterProjectionDistance = glm::dot(ray.direction, rayOriginToBsCenter);
            if (bsCenterProjectionDistance < 0.f) continue;

            if (std::abs(bsCenterProjectionDistance) > minDistance) continue;
            auto bsCenterProjection = ray.origin + ray.direction * bsCenterProjectionDistance;
            if (glm::length(bsCenter - bsCenterProjection) > bsRadius) continue;

            minDistance = bsCenterProjectionDistance;
            snappingInfo.point = &snappingPoint;
            snappingInfo.validForBrick = isSnappingPointValid(brick, snappingPoint);
        }
    }

    return snappingInfo;
}

void Panel::powered(bool powered)
{
    if (m_powered == powered) return;
    m_powered = powered;

    m_material->set("ghostFactor", (m_powered) ? 0.f : 1.f);
    m_borderMaterial->set("ghostFactor", (m_powered) ? 0.f : 1.f);
}

void Panel::onSolvedChanged(SolvedChangedCallback callback)
{
    m_solvedChangedCallbacks.emplace_back(callback);
}

void Panel::pretendSolved(bool pretendSolved)
{
    if (m_pretendSolved == pretendSolved) return;
    m_pretendSolved = pretendSolved;

    updateSolved();
}

void Panel::updateFromSnappedBricks()
{
    // Reset info about table snapping points filling.
    for (auto i = 0u; i < m_extent.x; ++i) {
        for (auto j = 0u; j < m_extent.y; ++j) {
            m_snappingPoints[i][j].hasBrickSnapped = false;
        }
    }

    for (auto brick : m_gameState.level.bricks) {
        if (&brick->snapPanel() != this) continue;

        for (const auto& block : brick->blocks()) {
            auto i = brick->snapCoordinates().x + block.coordinates.x;
            auto j = brick->snapCoordinates().y + block.coordinates.y;
            m_snappingPoints[i][j].hasBrickSnapped = true;
        }
    }
}

// Internal

void Panel::updateBorderMeshPrimitive()
{
    auto& meshComponent = m_entity->get<sill::MeshComponent>();

    const float halfWidth = blockExtent.x * m_extent.x / 2.f;
    const float halfHeight = blockExtent.y * m_extent.y / 2.f;
    const float halfDepth = blockExtent.z / 2.f;

    // @note Points are duplicated so that normals look nice.
    std::vector<glm::vec3> positions = {
        // BACK: Outer
        {-halfWidth - thickness, -halfHeight - thickness, -halfDepth},
        {halfWidth + thickness, -halfHeight - thickness, -halfDepth},
        {halfWidth + thickness, halfHeight + thickness, -halfDepth},
        {-halfWidth - thickness, halfHeight + thickness, -halfDepth},
        // FRONT: Inner
        {-halfWidth, -halfHeight, halfDepth},
        {halfWidth, -halfHeight, halfDepth},
        {halfWidth, halfHeight, halfDepth},
        {-halfWidth, halfHeight, halfDepth},
        // FRONT: Outer
        {-halfWidth - thickness, -halfHeight - thickness, halfDepth},
        {halfWidth + thickness, -halfHeight - thickness, halfDepth},
        {halfWidth + thickness, halfHeight + thickness, halfDepth},
        {-halfWidth - thickness, halfHeight + thickness, halfDepth},
        // LEFT: Outer
        {-halfWidth - thickness, -halfHeight - thickness, halfDepth},
        {-halfWidth - thickness, halfHeight + thickness, halfDepth},
        {-halfWidth - thickness, halfHeight + thickness, -halfDepth},
        {-halfWidth - thickness, -halfHeight - thickness, -halfDepth},
        // LEFT: Inner
        {-halfWidth, -halfHeight, halfDepth},
        {-halfWidth, halfHeight, halfDepth},
        {-halfWidth, halfHeight, -halfDepth},
        {-halfWidth, -halfHeight, -halfDepth},
        // TOP: Outer
        {-halfWidth - thickness, halfHeight + thickness, halfDepth},
        {halfWidth + thickness, halfHeight + thickness, halfDepth},
        {halfWidth + thickness, halfHeight + thickness, -halfDepth},
        {-halfWidth - thickness, halfHeight + thickness, -halfDepth},
        // TOP: Inner
        {-halfWidth, halfHeight, halfDepth},
        {halfWidth, halfHeight, halfDepth},
        {halfWidth, halfHeight, -halfDepth},
        {-halfWidth, halfHeight, -halfDepth},
        // RIGHT: Outer
        {halfWidth + thickness, -halfHeight - thickness, -halfDepth},
        {halfWidth + thickness, halfHeight + thickness, -halfDepth},
        {halfWidth + thickness, halfHeight + thickness, halfDepth},
        {halfWidth + thickness, -halfHeight - thickness, halfDepth},
        // RIGHT: Inner
        {halfWidth, -halfHeight, -halfDepth},
        {halfWidth, halfHeight, -halfDepth},
        {halfWidth, halfHeight, halfDepth},
        {halfWidth, -halfHeight, halfDepth},
        // BOTTOM: Outer
        {-halfWidth - thickness, -halfHeight - thickness, -halfDepth},
        {halfWidth + thickness, -halfHeight - thickness, -halfDepth},
        {halfWidth + thickness, -halfHeight - thickness, halfDepth},
        {-halfWidth - thickness, -halfHeight - thickness, halfDepth},
        // BOTTOM: Inner
        {-halfWidth, -halfHeight, -halfDepth},
        {halfWidth, -halfHeight, -halfDepth},
        {halfWidth, -halfHeight, halfDepth},
        {-halfWidth, -halfHeight, halfDepth},
    };

    std::vector<uint16_t> indices = {// BACK
                                     2, 1, 0, 0, 3, 2,
                                     // FRONT
                                     9, 5, 4, 4, 8, 9, 10, 6, 5, 5, 9, 10, 11, 7, 6, 6, 10, 11, 8, 4, 7, 7, 11, 8,
                                     // LEFT
                                     12, 13, 14, 14, 15, 12, 18, 17, 16, 16, 19, 18,
                                     // TOP
                                     20, 21, 22, 22, 23, 20, 26, 25, 24, 24, 27, 26,
                                     // RIGHT
                                     28, 29, 30, 30, 31, 28, 34, 33, 32, 32, 35, 34,
                                     // BOTTOM
                                     36, 37, 38, 38, 39, 36, 42, 41, 40, 40, 43, 42};

    auto& primitive = meshComponent.primitive(2, 0);
    primitive.verticesCount(positions.size());
    primitive.verticesPositions(positions);
    primitive.indices(indices);
    primitive.computeFlatNormals();
    primitive.computeTangents();
}

void Panel::updateSnappingPoints()
{
    if (m_extent.x == 0 || m_extent.y == 0) return;

    // Update snappingPoints
    glm::vec3 fextent(1.f - glm::vec2(m_extent), 0.f);
    lava::Transform originTransform;
    originTransform.translation = fextent * blockExtent / 2.f;

    m_snappingPoints.resize(m_extent.x);
    for (auto i = 0u; i < m_extent.x; ++i) {
        m_snappingPoints[i].resize(m_extent.y);
        for (auto j = 0u; j < m_extent.y; ++j) {
            lava::Transform localOriginTransform = originTransform;
            localOriginTransform.translation += glm::vec3(i * blockExtent.x, j * blockExtent.y, 0.f);
            m_snappingPoints[i][j].worldTransform =
                m_entity->get<sill::TransformComponent>().worldTransform() * localOriginTransform;
            m_snappingPoints[i][j].coordinates = {i, j};
            m_snappingPoints[i][j].hasBrickSnapped = false;
        }
    }
}

void Panel::updateSolved()
{
    if (!m_consolidated) return;

    updateFromSnappedBricks();

    bool solved = true;

    if (!m_pretendSolved) {
        // Check that the panel is filled.
        for (auto i = 0u; i < m_extent.x; ++i) {
            for (auto j = 0u; j < m_extent.y; ++j) {
                if (!m_snappingPoints[i][j].hasBrickSnapped) {
                    solved = false;
                    break;
                }
            }
        }
    }

    if (m_solved == solved) return;
    m_solved = solved;

    if (m_solved) {
        // Visual feedback: unsolved panels are white, and solved panels green.
        animation().start(sill::AnimationFlag::MaterialUniform, borderMaterial(), "albedoColor", 0.1f);
        animation().target(sill::AnimationFlag::MaterialUniform, borderMaterial(), "albedoColor", glm::vec4{0.46, 0.95, 0.46, 1.f});

        if (!m_substanceFreedOnSolve.empty()) {
            freeSubstance(m_gameState, m_substanceFreedOnSolve);
        }
        if (!m_substanceRevealedOnSolve.empty()) {
            revealSubstance(m_gameState, m_substanceRevealedOnSolve);
        }
    }
    else {
        animation().start(sill::AnimationFlag::MaterialUniform, borderMaterial(), "albedoColor", 0.5f);
        animation().target(sill::AnimationFlag::MaterialUniform, borderMaterial(), "albedoColor", glm::vec4{1.f});
    }

    for (auto& callback : m_solvedChangedCallbacks) {
        callback(m_solved);
    }
}

bool Panel::isSnappingPointValid(const Brick& brick, const SnappingPoint& snappingPoint)
{
    for (const auto& block : brick.blocks()) {
        auto i = snappingPoint.coordinates.x + block.coordinates.x;
        auto j = snappingPoint.coordinates.y + block.coordinates.y;

        // Check that the block coordinates are valid.
        if (i >= m_extent.x) return false;
        if (j >= m_extent.y) return false;

        // Check that the block coordinates have nothing yet.
        if (m_snappingPoints[i][j].hasBrickSnapped) return false;
    }

    return true;
}

// -----

Panel* findPanelByName(GameState& gameState, const std::string& name)
{
    for (auto panel : gameState.level.panels) {
        if (panel->name() == name) {
            return panel;
        }
    }

    return nullptr;
}

Panel* findPanel(GameState& gameState, const sill::Entity& entity)
{
    for (auto panel : gameState.level.panels) {
        if (&panel->entity() == &entity) {
            return panel;
        }
    }

    return nullptr;
}

uint32_t findPanelIndex(GameState& gameState, const sill::Entity& entity)
{
    for (auto i = 0u; i < gameState.level.panels.size(); ++i) {
        auto panel = gameState.level.panels[i];
        if (&panel->entity() == &entity) {
            return i;
        }
    }

    return -1u;
}
