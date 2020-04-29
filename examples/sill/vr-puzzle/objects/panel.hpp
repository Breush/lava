#pragma once

#include "./object.hpp"

#include <cstdint>
#include <glm/glm.hpp>
#include <lava/core/ray.hpp>
#include <vector>

class Brick;
class Barrier;

class Panel : public Object {
public:
    using SolvedChangedCallback = std::function<void(bool)>;

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
    void clear(bool removeFromLevel = true) final;

    const lava::sill::GameEntity& entity() const { return *m_entity; }
    lava::sill::GameEntity& entity() { return *m_entity; }

    const std::string& name() const { return m_name; }
    void name(const std::string& name) { m_name = name; }

    /// Update extent and associated materials visuals. This also resets all rules.
    const glm::uvec2& extent() const { return m_extent; }
    void extent(const glm::uvec2& extent);

    /// All barriers that allow the panel to be active.
    const std::set<Barrier*>& barriers() const { return m_barriers; }
    void addBarrier(Barrier& barrier) { m_barriers.emplace(&barrier); }
    void removeBarrier(Barrier& barrier);

    /// Checks whether the user is allowed to snap brick to this panel.
    bool userInteractionAllowed() const;

    /// 'from' and 'to' should be at distance 1.
    void addLink(const glm::uvec2& from, const glm::uvec2& to);

    /// Find the closest snapping point to is valid for the brick.
    SnappingPoint* closestSnappingPoint(const Brick& brick, const glm::vec3& position, float minDistance = 0.1f);

    /// Find the closest snapping point.
    SnappingInfo rayHitSnappingPoint(const Brick& brick, const lava::Ray& ray);

    /// Be warn whenever this panel solved status changed.
    void onSolvedChanged(SolvedChangedCallback callback);

    /// Whether this panel is currently solved.
    bool solved() const { return m_solved; }

    /// Hijack the system to make the game believe the panel is solved.
    void pretendSolved(bool pretendSolved);

    lava::magma::Material& borderMaterial() { return *m_borderMaterial; }

    // Callback used by Brick.
    void snappedBricksChanged() { updateSolved(); }

protected:
    void updateBorderMeshPrimitive();
    void updateSnappingPoints();
    void updateUniformData();
    void updateFromSnappedBricks();

    bool isSnappingPointValid(const Brick& brick, const SnappingPoint& snappingPoint);

    void updateSolved();

private:
    bool m_solved = false;
    bool m_pretendSolved = false;

    // Configuration
    std::string m_name;
    glm::uvec2 m_extent = {0u, 0u};
    std::set<Barrier*> m_barriers;
    std::vector<std::pair<glm::uvec2, glm::uvec2>> m_links;

    // Data
    std::vector<uint32_t> m_uniformData;
    std::vector<std::vector<SnappingPoint>> m_snappingPoints;

    // Mesh
    lava::magma::MaterialPtr m_material = nullptr;
    lava::magma::MaterialPtr m_borderMaterial = nullptr;

    std::vector<SolvedChangedCallback> m_solvedChangedCallbacks;
};

Panel* findPanelByName(GameState& gameState, const std::string& name);
Panel* findPanel(GameState& gameState, const lava::sill::GameEntity& entity);
uint32_t findPanelIndex(GameState& gameState, const lava::sill::GameEntity& entity);
