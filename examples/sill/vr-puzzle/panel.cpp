#include "./panel.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

#include "./brick.hpp"
#include "./game-state.hpp"
#include "./symbols.hpp"

using namespace lava;

Panel::Panel(GameState& gameState)
    : m_gameState(gameState)
{
    auto& engine = *gameState.engine;

    // @fixme This panel should be more abstract and not
    // hold the table stand mesh.

    // Set up panel material
    m_material = &engine.make<sill::Material>("panel");

    // Load and get table info
    m_entity = &engine.make<sill::GameEntity>();
    auto& meshComponent = m_entity->make<sill::MeshComponent>();
    sill::makers::glbMeshMaker("./assets/models/vr-puzzle/puzzle-table.glb")(meshComponent);
    m_entity->make<sill::AnimationComponent>();
    m_entity->get<sill::TransformComponent>().onWorldTransformChanged([this] { updateSnappingPoints(); });

    for (auto& node : meshComponent.nodes()) {
        if (node.name == "table") {
            // @todo As said above, we don't hold table stand material
            // because it should not even be our job creating it.
            m_tableMaterial = &node.mesh->primitive(0).material();
            continue;
        }

        // Setting panel material
        if (node.name == "panel") {
            m_meshNode = &node;
            node.mesh->primitive(0).material(*m_material);
            continue;
        }
    }
}

Panel::~Panel()
{
    m_gameState.engine->remove(*m_entity);
}

void Panel::extent(const glm::uvec2& extent)
{
    m_extent = extent;
    m_material->set("extent", extent);

    // Reset rules
    m_lastKnownSolveStatus = true;
    m_links.clear();

    m_uniformData.resize(extent.x * extent.y);
    updateUniformData();

    updateSnappingPoints();
    updateFromSnappedBricks();
}

void Panel::addLink(const glm::uvec2& from, const glm::uvec2& to)
{
    m_lastKnownSolveStatus = true;
    m_links.emplace_back(from, to);
    updateUniformData();
}

/// Find the closest MeshNode for the table snapping points.
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

bool Panel::checkSolveStatus(bool* solveStatusChanged)
{
    updateFromSnappedBricks();

    // Check that the panel is filled.
    for (auto i = 0u; i < m_extent.x; ++i) {
        for (auto j = 0u; j < m_extent.y; ++j) {
            if (!m_snappingPoints[i][j].hasBrickSnapped) {
                if (solveStatusChanged) *solveStatusChanged = (m_lastKnownSolveStatus != false);
                m_lastKnownSolveStatus = false;
                return false;
            }
        }
    }

    // Check that all links are correct.
    for (const auto& link : m_links) {
        const auto& from = link.first;
        const auto& to = link.second;

        // Find the brick that has 'from'.
        const Brick* fromBrick = nullptr;
        for (const auto& brick : m_gameState.bricks) {
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
            if (solveStatusChanged) *solveStatusChanged = (m_lastKnownSolveStatus != false);
            m_lastKnownSolveStatus = false;
            return false;
        }
    }

    if (solveStatusChanged) *solveStatusChanged = (m_lastKnownSolveStatus != true);
    m_lastKnownSolveStatus = true;
    return true;
}

void Panel::updateFromSnappedBricks()
{
    // Reset info about table snapping points filling.
    for (auto i = 0u; i < m_extent.x; ++i) {
        for (auto j = 0u; j < m_extent.y; ++j) {
            m_snappingPoints[i][j].hasBrickSnapped = false;
        }
    }

    for (auto& brick : m_gameState.bricks) {
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

void Panel::updateSnappingPoints()
{
    if (m_extent.x == 0 || m_extent.y == 0) return;

    // Update snappingPoints
    glm::vec3 fextent(1.f - glm::vec2(m_extent), 0.f);

    // Setting local transform for the brick to snap nicely
    glm::mat4 originTransform = glm::mat4(1.f);
    originTransform[3] = {0, 0, 1.205f, 1}; // @note Panel height, as defined within model
    originTransform = glm::rotate(originTransform, 3.14156f * 0.5f, {0, 0, 1});
    originTransform = glm::rotate(originTransform, 3.14156f * 0.375f, {1, 0, 0});
    originTransform = glm::translate(originTransform, fextent * blockExtent / 2.f);

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
