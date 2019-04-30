#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <lava/sill.hpp>
#include <vector>

class Brick;
class GameState;

class Panel {
public:
    struct BindingPoint {
        glm::mat4 worldTransform = glm::mat4(1.f);
        glm::uvec2 coordinates = glm::uvec2(-1u, -1u);
        bool hasBrickSnapped = false;
    };

public:
    /// Load meshes and materials. Should be called before extent().
    void init(GameState& gameState);

    /// Update extent and associated materials visuals. This also resets all rules.
    void extent(const glm::uvec2& extent);

    /// 'from' and 'to' should be at distance 1.
    void addLink(const glm::uvec2& from, const glm::uvec2& to);

    bool checkSolveStatus(GameState& gameState);
    void updateFromSnappedBricks(GameState& gameState);

    /// Find the closest MeshNode for the table binding points.
    BindingPoint* closestBindingPoint(const Brick& brick, const glm::vec3& position, float minDistance = 0.1f);

protected:
    void updateUniformData();

    bool isBindingPointValid(const Brick& brick, const BindingPoint& bindingPoint);

private:
    // Configuration
    glm::uvec2 m_extent = {0u, 0u};
    std::vector<std::pair<glm::uvec2, glm::uvec2>> m_links;

    // Data
    std::vector<uint32_t> m_uniformData;
    std::vector<std::vector<BindingPoint>> m_bindingPoints;

    // Mesh
    lava::sill::GameEntity* m_entity = nullptr;
    lava::sill::MeshNode* m_meshNode = nullptr;
    lava::sill::Material* m_material = nullptr;
};
