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

    // Editor controls
    void uiWidgets(std::vector<UiWidget>& widgets) final;

    /// Update extent and associated materials visuals. This also resets all rules.
    const glm::uvec2& extent() const { return m_extent; }
    void extent(const glm::uvec2& extent);

    /// Checks whether the user is allowed to snap brick to this panel.
    bool userInteractionAllowed() const;

    /// Find the closest snapping point to is valid for the brick.
    SnappingPoint* closestSnappingPoint(const Brick& brick, const glm::vec3& position, float minDistance = 0.1f);

    /// Find the closest snapping point.
    SnappingInfo rayHitSnappingPoint(const Brick& brick, const lava::Ray& ray);

    /// An un-powered panel is black and has no interaction.
    bool powered() const { return m_powered; }
    void powered(bool powered);

    const std::string& substanceRevealNeeded() const { return m_substanceRevealNeeded; }
    void substanceRevealNeeded(const std::string& substanceRevealNeeded) { m_substanceRevealNeeded = substanceRevealNeeded; }
    const std::string& substanceFreedOnSolve() const { return m_substanceFreedOnSolve; }
    void substanceFreedOnSolve(const std::string& substanceFreedOnSolve) { m_substanceFreedOnSolve = substanceFreedOnSolve; }
    const std::string& substanceRevealedOnSolve() const { return m_substanceRevealedOnSolve; }
    void substanceRevealedOnSolve(const std::string& substanceRevealedOnSolve) { m_substanceRevealedOnSolve = substanceRevealedOnSolve; }

    /// Be warn whenever this panel solved status changed.
    void onSolvedChanged(SolvedChangedCallback callback);

    /// Whether this panel is currently solved.
    bool solved() const { return m_solved; }

    /// Hijack the system to make the game believe the panel is solved.
    bool pretendSolved() const { return m_pretendSolved; }
    void pretendSolved(bool pretendSolved);

    lava::magma::Material& borderMaterial() { return *m_borderMaterial; }

    // Callback used by Brick.
    void snappedBricksChanged() { updateSolved(); }

protected:
    void updateBorderMeshPrimitive();
    void updateSnappingPoints();
    void updateFromSnappedBricks();

    bool isSnappingPointValid(const Brick& brick, const SnappingPoint& snappingPoint);

    void updateSolved();

private:
    bool m_consolidated = false;

    bool m_powered = false;
    bool m_solved = false;
    bool m_pretendSolved = false;

    // Configuration
    glm::uvec2 m_extent = {3u, 3u};
    std::string m_substanceRevealNeeded;
    std::string m_substanceFreedOnSolve;
    std::string m_substanceRevealedOnSolve;

    // Data
    std::vector<std::vector<SnappingPoint>> m_snappingPoints;

    // Mesh
    lava::magma::MaterialPtr m_material = nullptr;
    lava::magma::MaterialPtr m_borderMaterial = nullptr;

    std::vector<SolvedChangedCallback> m_solvedChangedCallbacks;
};

Panel* findPanelByName(GameState& gameState, const std::string& name);
Panel* findPanel(GameState& gameState, const lava::sill::Entity& entity);
uint32_t findPanelIndex(GameState& gameState, const lava::sill::Entity& entity);
