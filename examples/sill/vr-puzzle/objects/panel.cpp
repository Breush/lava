#include "./panel.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

#include "./brick.hpp"
#include "../game-state.hpp"
#include "../symbols.hpp"

using namespace lava;

namespace {
    constexpr const float thickness = 0.1f;
}

Panel::Panel(GameState& gameState)
    : Object(gameState)
{
    auto& engine = *gameState.engine;

    // Create panel
    m_entity = &engine.make<sill::GameEntity>("panel");
    auto& meshComponent = m_entity->make<sill::MeshComponent>();
    sill::makers::PlaneMeshOptions options;
    options.rootNodeHasGeometry = false;
    sill::makers::planeMeshMaker({blockExtent.x, blockExtent.y}, options)(meshComponent);
    m_material = engine.scene().makeMaterial("panel");
    meshComponent.primitive(1, 0).material(m_material);

    // Create border
    auto& meshNode = meshComponent.addNode();
    meshNode.name = "border";
    meshNode.meshGroup = std::make_unique<sill::MeshGroup>(meshComponent.scene());
    meshNode.parent = &meshComponent.node(0);
    meshComponent.node(0).children.emplace_back(&meshNode);
    m_borderMaterial = engine.scene().makeMaterial("roughness-metallic");
    meshNode.meshGroup->addPrimitive().material(m_borderMaterial);

    m_entity->make<sill::ColliderComponent>();
    m_entity->get<sill::PhysicsComponent>().dynamic(false);
    m_entity->make<sill::AnimationComponent>();
    m_entity->get<sill::TransformComponent>().onWorldTransformChanged([this] { updateSnappingPoints(); });
}

void Panel::clear(bool removeFromLevel)
{
    Object::clear(removeFromLevel);

    if (removeFromLevel) {
        for (auto& brick : m_gameState.level.bricks) {
            if (brick->snapped() && &brick->snapPanel() == this) {
                brick->unsnap();
            }
        }

        auto panelIt = std::find_if(m_gameState.level.panels.begin(), m_gameState.level.panels.end(), [this](const std::unique_ptr<Panel>& panel) {
            return (panel.get() == this);
        });
        m_gameState.level.panels.erase(panelIt);
    }
}

void Panel::removeBarrier(Barrier& barrier)
{
    auto barrierIt = m_barriers.find(&barrier);
    if (barrierIt == m_barriers.end()) return;
    m_barriers.erase(barrierIt);
}

bool Panel::userInteractionAllowed() const
{
    auto playerPosition = glm::vec2(m_gameState.player.position);
    for (auto barrier : m_barriers) {
        if (!barrier->powered()) return false;

        auto barrierPosition = glm::vec2(barrier->transform().translation());
        if (glm::distance(playerPosition, barrierPosition) >= barrier->diameter() / 2.f) {
            return false;
        }
    }

    return true;
}

void Panel::extent(const glm::uvec2& extent)
{
    m_extent = extent;
    m_material->set("extent", extent);
    m_entity->get<sill::MeshComponent>().node(1).transform(glm::scale(glm::mat4(1.f), {m_extent.x, m_extent.y, 1}));

    m_entity->get<sill::ColliderComponent>().clearShapes();
    m_entity->get<sill::ColliderComponent>().addBoxShape({0.f, 0.f, 0.f},
        {m_extent.x * blockExtent.x + 2.f * thickness,
         m_extent.y * blockExtent.y + 2.f * thickness,
         blockExtent.z});

    updateBorderMeshPrimitive();

    // Reset rules
    m_links.clear();

    m_uniformData.resize(extent.x * extent.y);
    updateUniformData();

    updateSnappingPoints();
    updateSolved();
}

void Panel::addLink(const glm::uvec2& from, const glm::uvec2& to)
{
    m_links.emplace_back(from, to);
    updateUniformData();
}

Panel::SnappingPoint* Panel::closestSnappingPoint(const Brick& brick, const glm::vec3& position, float minDistance)
{
    updateFromSnappedBricks();

    SnappingPoint* closestSnappingPoint = nullptr;

    for (auto& snappingPoints : m_snappingPoints) {
        for (auto& snappingPoint : snappingPoints) {
            auto distance = glm::distance(glm::vec3(snappingPoint.worldTransform[3]), position);
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
            glm::vec3 bsCenter = snappingPoint.worldTransform[3];
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

void Panel::onSolve(std::function<void()> callback)
{
    m_solveCallbacks.emplace_back(callback);
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

    for (auto& brick : m_gameState.level.bricks) {
        if (!brick->snapped()) continue;
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
    glm::mat4 originTransform = glm::translate(glm::mat4(1.f), fextent * blockExtent / 2.f);

    m_snappingPoints.resize(m_extent.x);
    for (auto i = 0u; i < m_extent.x; ++i) {
        m_snappingPoints[i].resize(m_extent.y);
        for (auto j = 0u; j < m_extent.y; ++j) {
            m_snappingPoints[i][j].worldTransform =
                m_entity->get<sill::TransformComponent>().worldTransform()
                * glm::translate(originTransform, glm::vec3(i * blockExtent.x, j * blockExtent.y, 0.f));
            m_snappingPoints[i][j].coordinates = {i, j};
            m_snappingPoints[i][j].hasBrickSnapped = false;
        }
    }
}

void Panel::updateUniformData()
{
    // Cleaning previous
    for (auto i = 0u; i < m_extent.x * m_extent.y; ++i) {
        m_uniformData[i] = 0u;
    }

    // Update from links
    for (const auto& link : m_links) {
        const auto& from = link.first;
        const auto& to = link.second;
        auto fromIndex = m_extent.y * from.x + from.y;
        auto toIndex = m_extent.y * to.x + to.y;

        m_uniformData[fromIndex] |= LINK_FLAG;
        m_uniformData[toIndex] |= LINK_FLAG;

        // EAST
        if (from.x < to.x) {
            m_uniformData[fromIndex] |= LINK_EAST_FLAG;
            m_uniformData[toIndex] |= LINK_WEST_FLAG;
        }
        // NORTH
        else if (from.y < to.y) {
            m_uniformData[fromIndex] |= LINK_NORTH_FLAG;
            m_uniformData[toIndex] |= LINK_SOUTH_FLAG;
        }
        // WEST
        else if (from.x > to.x) {
            m_uniformData[fromIndex] |= LINK_WEST_FLAG;
            m_uniformData[toIndex] |= LINK_EAST_FLAG;
        }
        // SOUTH
        else if (from.y > to.y) {
            m_uniformData[fromIndex] |= LINK_SOUTH_FLAG;
            m_uniformData[toIndex] |= LINK_NORTH_FLAG;
        }
    }

    m_material->set("symbols", m_uniformData.data(), m_uniformData.size());
}

void Panel::updateSolved()
{
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

        // Check that all links are correct.
        for (const auto& link : m_links) {
            const auto& from = link.first;
            const auto& to = link.second;

            // Find the brick that has 'from'.
            const Brick* fromBrick = nullptr;
            for (const auto& brick : m_gameState.level.bricks) {
                for (const auto& block : brick->blocks()) {
                    if (brick->snapCoordinates().x + block.coordinates.x == from.x
                        && brick->snapCoordinates().y + block.coordinates.y == from.y) {
                        fromBrick = brick.get();
                        break;
                    }
                }
            }

            // Check that it also has 'to'.
            bool foundTo = false;
            for (const auto& block : fromBrick->blocks()) {
                if (fromBrick->snapCoordinates().x + block.coordinates.x == to.x
                    && fromBrick->snapCoordinates().y + block.coordinates.y == to.y) {
                    foundTo = true;
                    break;
                }
            }

            if (!foundTo) {
                // @fixme Add visual feedback, explaining why this has failed
                solved = false;
            }
        }
    }

    if (m_solved == solved) return;
    m_solved = solved;

    if (m_solved) {
        // Visual feedback: unsolved panels are white, and solved panels green.
        animation().start(sill::AnimationFlag::MaterialUniform, borderMaterial(), "albedoColor", 0.1f);
        animation().target(sill::AnimationFlag::MaterialUniform, borderMaterial(), "albedoColor", glm::vec4{0.46, 0.95, 0.46, 1.f});

        for (auto& callback : m_solveCallbacks) {
            callback();
        }
    }
    else {
        animation().start(sill::AnimationFlag::MaterialUniform, borderMaterial(), "albedoColor", 0.5f);
        animation().target(sill::AnimationFlag::MaterialUniform, borderMaterial(), "albedoColor", glm::vec4{1.f});
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
    for (const auto& panel : gameState.level.panels) {
        if (panel->name() == name) {
            return panel.get();
        }
    }

    return nullptr;
}

Panel* findPanel(GameState& gameState, const sill::GameEntity& entity)
{
    for (const auto& panel : gameState.level.panels) {
        if (&panel->entity() == &entity) {
            return panel.get();
        }
    }

    return nullptr;
}

uint32_t findPanelIndex(GameState& gameState, const sill::GameEntity& entity)
{
    for (auto i = 0u; i < gameState.level.panels.size(); ++i) {
        if (&gameState.level.panels[i]->entity() == &entity) {
            return i;
        }
    }

    return -1u;
}
