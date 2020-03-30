#pragma once

#include <lava/sill/components/camera-component.hpp>

#include "./component-impl.hpp"

namespace lava::sill {
    class CameraComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);
        ~Impl();

        // IComponent
        void update(float dt) override final;

        // CameraComponent
        const Extent2d& extent() const { return m_extent; }

        const glm::vec3& origin() const { return m_cameraController.origin(); }
        void origin(const glm::vec3& origin) { m_cameraController.origin(origin); }

        const glm::vec3& target() const { return m_cameraController.target(); }
        void target(const glm::vec3& target) { m_cameraController.target(target); }

        float radius() const { return m_cameraController.radius(); }
        void radius(float radius) { m_cameraController.radius(radius); }

        void strafe(float x, float y) { m_cameraController.strafe(x, y); }
        void radiusAdd(float radiusDistance) { m_cameraController.radiusAdd(radiusDistance); }
        void orbitAdd(float longitudeAngle, float latitudeAngle) { m_cameraController.orbitAdd(longitudeAngle, latitudeAngle); }
        void rotateAtOrigin(float longitudeAngle, float latitudeAngle) { m_cameraController.rotateAtOrigin(longitudeAngle, latitudeAngle); }

        const glm::mat4& viewTransform() const { return m_camera->viewTransform(); }
        const glm::mat4& projectionTransform() const { return m_camera->projectionTransform(); }

        Ray coordinatesToRay(const glm::vec2& coordinates, float depth = 0.f) const;
        glm::vec3 unproject(const glm::vec2& coordinates, float depth = 0.f) const;
        magma::Frustum frustum(const glm::vec2& topLeftCoordinates, const glm::vec2& bottomRightCoordinates) const;

    private:
        magma::Camera* m_camera = nullptr;
        magma::OrbitCameraController m_cameraController;

        Extent2d m_extent;
        float m_updateDelay = -1.f;
    };
}
