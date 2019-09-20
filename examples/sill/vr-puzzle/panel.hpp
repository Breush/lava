#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <lava/core/ray.hpp>
#include <lava/sill.hpp>
#include <vector>

class Brick;
struct GameState;

class Panel {
public:
    struct SnappingPoint {
        glm::mat4 worldTransform = glm::mat4(1.f);
        glm::uvec2 coordinates = glm::uvec2(-1u, -1u);
        bool hasBrickSnapped = false;
    };

    struct SnappingInfo {
        SnappingPoint* point = nullptr;
        bool validForBrick = false;
    };

public:
    Panel(GameState& gameState);
    ~Panel();

    const lava::sill::AnimationComponent& animation() const { return m_entity->get<lava::sill::AnimationComponent>(); };
    lava::sill::AnimationComponent& animation() { return m_entity->get<lava::sill::AnimationComponent>(); };
    const lava::sill::TransformComponent& transform() const { return m_entity->get<lava::sill::TransformComponent>(); };
    lava::sill::TransformComponent& transform() { return m_entity->get<lava::sill::TransformComponent>(); };

    const std::string& name() const { return m_name; }
    void name(const std::string& name) { m_name = name; }

    /// Update extent and associated materials visuals. This also resets all rules.
    const glm::uvec2& extent() const { return m_extent; }
    void extent(const glm::uvec2& extent);

    /// 'from' and 'to' should be at distance 1.
    void addLink(const glm::uvec2& from, const glm::uvec2& to);

    /// Find the closest snapping point to is valid for the brick.
    SnappingPoint* closestSnappingPoint(const Brick& brick, const glm::vec3& position, float minDistance = 0.1f);

    /// Find the closest snapping point.
    SnappingInfo rayHitSnappingPoint(const Brick& brick, const lava::Ray& ray);

    /// Check and update if panel is completely solved.
    bool checkSolveStatus(bool* solveStatusChanged);

    /// Be warn whenever this panel goes from unsolve to solve status after a checkSolveStatus().
    void onSolve(std::function<void()> callback);

    // @fixme As said above, table stand should not be of our concern in the end.
    lava::magma::Material& tableMaterial() { return *m_tableMaterial; }

protected:
    void updateSnappingPoints();
    void updateUniformData();
    void updateFromSnappedBricks();

    bool isSnappingPointValid(const Brick& brick, const SnappingPoint& snappingPoint);

private:
    GameState& m_gameState;

    bool m_lastKnownSolveStatus = true;

    // Configuration
    std::string m_name;
    glm::uvec2 m_extent = {0u, 0u};
    std::vector<std::pair<glm::uvec2, glm::uvec2>> m_links;

    // Data
    std::vector<uint32_t> m_uniformData;
    std::vector<std::vector<SnappingPoint>> m_snappingPoints;

    // Mesh
    lava::sill::GameEntity* m_entity = nullptr;
    lava::sill::MeshNode* m_meshNode = nullptr;
    lava::magma::Material* m_material = nullptr;

    lava::magma::Material* m_tableMaterial = nullptr;

    std::vector<std::function<void()>> m_solveCallbacks;
};

Panel& findPanelByName(GameState& gameState, const std::string& name);
