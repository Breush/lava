#pragma once

#include "./generic.hpp"

#include <cstdint>
#include <glm/glm.hpp>
#include <lava/core/ray.hpp>
#include <vector>

class Brick;
class Barrier;

class Panel : public Generic {
public:
    using SolvedChangedCallback = std::function<void(bool)>;

    struct SnappingPoint {
        lava::Transform worldTransform;
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

    void unserialize(const nlohmann::json& data) final;
    nlohmann::json serialize() const final;
    void consolidateReferences() final;

    /// Update extent and associated materials visuals. This also resets all rules.
    const glm::uvec2& extent() const { return m_extent; }
    void extent(const glm::uvec2& extent);

    /// All barriers that allow the panel to be active.
    void addBarrier(Barrier& barrier);
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

protected:
    struct BarrierInfo {
        // @note :UnconsolidatedId
        uint32_t unconsolidatedBarrierId = -1u;
        Barrier* barrier = nullptr;
    };

private:
    bool m_consolidated = false;

    bool m_solved = false;
    bool m_pretendSolved = false;

    // Configuration
    glm::uvec2 m_extent = {3u, 3u};
    std::vector<BarrierInfo> m_barrierInfos;
    std::vector<std::pair<glm::uvec2, glm::uvec2>> m_links; // @todo Deprecated because unused?

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
