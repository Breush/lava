#include "./panel.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

#include "./brick.hpp"
#include "./game-state.hpp"
#include "./symbols.hpp"

using namespace lava;

void Panel::init(GameState& gameState)
{
    auto& engine = *gameState.engine;

    // @fixme This panel should be more abstract and not
    // hold the table stand mesh.

    // Set up panel material
    m_material = &engine.make<sill::Material>("panel");

    // Load and get table info
    auto& entity = engine.make<sill::GameEntity>();
    auto& meshComponent = entity.make<sill::MeshComponent>();
    sill::makers::glbMeshMaker("./assets/models/vr-puzzle/puzzle-table.glb")(meshComponent);
    entity.get<sill::TransformComponent>().rotate({0, 0, 1}, 3.14156f);
    entity.make<sill::AnimationComponent>();
    // @todo As said above, tableEntity should be of our concern in the end.
    gameState.tableEntity = &entity;
    m_entity = &entity;

    for (auto& node : meshComponent.nodes()) {
        if (node.name == "table") {
            // @todo As said above, we don't hold table stand material
            // because it should not even be our job creating it.
            gameState.tableMaterial = &node.mesh->primitive(0).material();
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

void Panel::extent(const glm::uvec2& extent)
{
    m_extent = extent;
    m_material->set("extent", extent);

    // Update bindingPoints
    glm::vec3 fextent(1.f - glm::vec2(extent), 0.f);

    // Setting local transform for the brick to snap nicely
    glm::mat4 originTransform = glm::mat4(1.f);
    originTransform[3] = {0, 0, 1.205f, 1}; // @note Panel height, as defined within model
    originTransform = glm::rotate(originTransform, 3.14156f * 0.5f, {0, 0, 1});
    originTransform = glm::rotate(originTransform, 3.14156f * 0.375f, {1, 0, 0});
    originTransform = glm::translate(originTransform, fextent * brickExtent / 2.f);

    m_bindingPoints.resize(extent.x);
    for (auto i = 0u; i < extent.x; ++i) {
        m_bindingPoints[i].resize(extent.y);
        for (auto j = 0u; j < extent.y; ++j) {
            m_bindingPoints[i][j].worldTransform =
                m_entity->get<sill::TransformComponent>().worldTransform()
                * glm::translate(originTransform, glm::vec3(i * brickExtent.x, j * brickExtent.y, 0.f));
            m_bindingPoints[i][j].coordinates = {i, j};
            m_bindingPoints[i][j].hasBrickSnapped = false;
        }
    }

    // Reset rules
    m_links.clear();

    m_uniformData.resize(extent.x * extent.y);
    updateUniformData();
}

void Panel::addLink(const glm::uvec2& from, const glm::uvec2& to)
{
    m_links.emplace_back(from, to);
    updateUniformData();
}

/// Find the closest MeshNode for the table binding points.
Panel::BindingPoint* Panel::closestBindingPoint(const Brick& brick, const glm::vec3& position, float minDistance)
{
    BindingPoint* closestBindingPoint = nullptr;

    for (auto& bindingPoints : m_bindingPoints) {
        for (auto& bindingPoint : bindingPoints) {
            auto distance = glm::distance(glm::vec3(bindingPoint.worldTransform[3]), position);
            if (distance < minDistance) {
                if (!isBindingPointValid(brick, bindingPoint)) continue;
                minDistance = distance;
                closestBindingPoint = &bindingPoint;
            }
        }
    }

    return closestBindingPoint;
}

bool Panel::checkSolveStatus(GameState& gameState)
{
    updateFromSnappedBricks(gameState);

    // Check that the panel is filled.
    for (auto i = 0u; i < m_extent.x; ++i) {
        for (auto j = 0u; j < m_extent.y; ++j) {
            if (!m_bindingPoints[i][j].hasBrickSnapped) {
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
        for (const auto& brick : gameState.bricks) {
            for (const auto& block : brick.blocks) {
                if (brick.snapCoordinates.x + block.x == from.x && brick.snapCoordinates.y + block.y == from.y) {
                    fromBrick = &brick;
                    break;
                }
            }
        }

        // Check that it also has 'to'.
        bool foundTo = false;
        for (const auto& block : fromBrick->blocks) {
            if (fromBrick->snapCoordinates.x + block.x == to.x && fromBrick->snapCoordinates.y + block.y == to.y) {
                foundTo = true;
                break;
            }
        }

        if (!foundTo) {
            // @fixme Add visual feedback, explaining why this has failed
            return false;
        }
    }

    return true;
}

void Panel::updateFromSnappedBricks(GameState& gameState)
{
    // Reset info about table binding points filling.
    for (auto i = 0u; i < m_extent.x; ++i) {
        for (auto j = 0u; j < m_extent.y; ++j) {
            m_bindingPoints[i][j].hasBrickSnapped = false;
        }
    }

    for (auto& brick : gameState.bricks) {
        if (!brick.snapped) continue;

        for (const auto& block : brick.blocks) {
            auto i = brick.snapCoordinates.x + block.x;
            auto j = brick.snapCoordinates.y + block.y;
            m_bindingPoints[i][j].hasBrickSnapped = true;
        }
    }
}

// Internal

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

bool Panel::isBindingPointValid(const Brick& brick, const BindingPoint& bindingPoint)
{
    for (const auto& block : brick.blocks) {
        auto i = bindingPoint.coordinates.x + block.x;
        auto j = bindingPoint.coordinates.y + block.y;

        // Check that the block coordinates are valid.
        if (i >= m_extent.x) return false;
        if (j >= m_extent.y) return false;

        // Check that the block coordinates have nothing yet.
        if (m_bindingPoints[i][j].hasBrickSnapped) return false;
    }

    return true;
}
