#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

class GameState;

class Panel {
public:
    Panel(uint32_t width, uint32_t height);

    // The panel rules as taken as input by the panel's material.
    const std::vector<uint32_t>& uniformData() const { return m_uniformData; }

    // from and to should be at distance 1
    void addLink(const glm::uvec2& from, const glm::uvec2& to);

    bool checkSolveStatus(GameState& gameState);

protected:
    void updateFillingInfo(GameState& gameState);
    void updateUniformData();

private:
    uint32_t m_width = 0u;
    uint32_t m_height = 0u;

    std::vector<uint32_t> m_uniformData;
    std::vector<std::pair<glm::uvec2, glm::uvec2>> m_links;
};
