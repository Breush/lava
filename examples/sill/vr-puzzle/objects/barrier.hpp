#pragma once

#include "./object.hpp"

#include <glm/glm.hpp>

/**
 * A barrier prevents teleportation or ray-picking (or both).
 * The effect goes infinitly high.
 */
class Barrier final : public Object {
public:
    struct ControlPoint {
        glm::vec2 point; // Control point per se, local coordinates.
        glm::vec3 position; // Automatically computed, based on terrain, absolute coordinates.
    };

public:
    Barrier(GameState& gameState);
    void clear(bool removeFromLevel = true) final;

    void unserialize(const nlohmann::json& data) final;
    nlohmann::json serialize() const final;
    void consolidateReferences() final;

    float halfSpan() const final { return 1.f; } // @todo Well... Hard to answer that.

    // Editor controls
    void editorOnClicked(const glm::vec3& hitPoint) final;
    const glm::vec3& editorOrigin() const final;
    void editorTranslate(const glm::vec3& translation) final;

    /// Fast check to know if a barrier is hit.
    bool intersectSegment(const glm::vec3& start, const glm::vec3& end) const;

    /// Always a positive distance, or 0.f if not hit.
    float distanceFrom(const lava::Ray& ray) const;

    /// An un-powered barrier is disabled and does not prevent anything.
    bool powered() const { return m_powered; }
    void powered(bool powered);

    bool pickingBlocked() const { return m_powered && m_pickingBlocked; }
    void pickingBlocked(bool pickingBlocked);
    bool teleportationBlocked() const { return m_powered && m_teleportationBlocked; }
    void teleportationBlocked(bool teleportationBlocked);

    /// A control point is a (x, y) point (z is determined based on terrain).
    /// Two consecutive control points in this list form a barrier-pane.
    const std::vector<ControlPoint>& controlPoints() const { return m_controlPoints; }

protected:
    void updateFromControlPoints();

protected:
    struct Pane {
        ControlPoint& p0;
        ControlPoint& p1;
        glm::vec3 direction; // Normalized projected direction (p1 - p0) on (X,Y) plane (no Z component)
        glm::vec3 normal;    // Normalized normal of pane plane (no Z component)
        float length2d;

        Pane(ControlPoint& inP0, ControlPoint& inP1)
            : p0(inP0), p1(inP1)
        {
            direction = glm::normalize(glm::vec3{p1.point.x - p0.point.x, p1.point.y - p0.point.y, 0.f});
            normal = glm::vec3{direction.y, -direction.x, 0.f};
            length2d = glm::length(p1.point - p0.point);
        }
    };

private:
    bool m_powered = false;
    bool m_pickingBlocked = true;
    bool m_teleportationBlocked = true;

    uint32_t m_editorControlPointIndex = 0u;

    lava::magma::MaterialPtr m_panesMaterial = nullptr;

    std::vector<ControlPoint> m_controlPoints;
    std::vector<Pane> m_panes;
};

Barrier* findBarrierByName(GameState& gameState, const std::string& name);
Barrier* findBarrier(GameState& gameState, const lava::sill::GameEntity& entity);
uint32_t findBarrierIndex(GameState& gameState, const lava::sill::GameEntity& entity);
