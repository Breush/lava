#include "./panel.hpp"

#include "./game-state.hpp"
#include "./symbols.hpp"

Panel::Panel(uint32_t width, uint32_t height)
    : m_width(width)
    , m_height(height)
{
    m_uniformData.resize(width * height, 0u);
}

void Panel::addLink(const glm::uvec2& from, const glm::uvec2& to)
{
    m_links.emplace_back(from, to);

    updateUniformData();
}

bool Panel::checkSolveStatus(GameState& gameState)
{
    updateFillingInfo(gameState);

    // Check that the panel is filled.
    for (auto i = 0u; i < gameState.tableBindingPoints.size(); ++i) {
        for (auto j = 0u; j < gameState.tableBindingPoints[i].size(); ++j) {
            if (!gameState.tableBindingPoints[i][j].filled) {
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

// Internal

void Panel::updateFillingInfo(GameState& gameState)
{
    // Reset info about table binding points filling.
    for (auto i = 0u; i < gameState.tableBindingPoints.size(); ++i) {
        for (auto j = 0u; j < gameState.tableBindingPoints[i].size(); ++j) {
            gameState.tableBindingPoints[i][j].filled = false;
        }
    }

    for (auto& brick : gameState.bricks) {
        if (!brick.snapped) continue;

        for (const auto& block : brick.blocks) {
            auto i = brick.snapCoordinates.x + block.x;
            auto j = brick.snapCoordinates.y + block.y;
            gameState.tableBindingPoints[i][j].filled = true;
        }
    }
}

void Panel::updateUniformData()
{
    // Cleaning previous
    for (auto i = 0u; i < m_width * m_height; ++i) {
        m_uniformData[i] = 0u;
    }

    // Update from links
    for (const auto& link : m_links) {
        const auto& from = link.first;
        const auto& to = link.second;
        auto fromIndex = m_height * from.x + from.y;
        auto toIndex = m_height * to.x + to.y;

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
}
