#pragma once

#include <lava/sill/components/i-component.hpp>

#include <glm/vec3.hpp>
#include <lava/core/extent.hpp>
#include <lava/core/ray.hpp>
#include <lava/magma/frustum.hpp>
#include <lava/magma/camera-controllers/orbit-camera-controller.hpp>
#include <string>

namespace lava::magma {
    class Camera;
}

namespace lava::sill {
    class CameraComponent final : public IComponent {
    public:
        CameraComponent(Entity& entity);
        ~CameraComponent();

        // IComponent
        static std::string hrid() { return "camera"; }

        const magma::Camera& camera() const { return *m_camera; }
        magma::Camera& camera() { return *m_camera; }

        /**
         * @name Configuration
         */
        /// @{
        /// Kept in sync with the rendering window.
        const Extent2d& extent() const { return m_extent; }
        /// @}

        /**
         * @name Controls
         */
        /// @{
        // Orbit
        const glm::vec3& origin() const { return m_cameraController.origin(); }
        void origin(const glm::vec3& origin) { m_cameraController.origin(origin); }
        const glm::vec3& target() const { return m_cameraController.target(); }
        void target(const glm::vec3& target) { m_cameraController.target(target); }
        float radius() const { return m_cameraController.radius(); }
        void radius(float radius) { m_cameraController.radius(radius); }

        void radiusAdd(float radiusDistance) { m_cameraController.radiusAdd(radiusDistance); }
        void orbitAdd(float longitudeAngle, float latitudeAngle){ m_cameraController.orbitAdd(longitudeAngle, latitudeAngle); }

        // FPS
        void go(const glm::vec3& origin); // Move both origin and target so that the camera seem to have moved there.
        void goForward(float distance, const glm::vec3& constraints = {1, 1, 1});
        void goRight(float distance, const glm::vec3& constraints = {1, 1, 1});
        void strafe(float x, float y) { m_cameraController.strafe(x, y); }
        void rotateAtOrigin(float longitudeAngle, float latitudeAngle) { m_cameraController.rotateAtOrigin(longitudeAngle, latitudeAngle); }
        /// @}

        /**
         * @name Transforms
         */
        /// @{
        /// Unproject screen-space coordinates to a 3D position.
        glm::vec3 unproject(const glm::vec2& coordinates, float depth = 0.f) const;
        /// Unproject screen-space coordinates to a ray going forward.
        Ray unprojectAsRay(const glm::vec2& coordinates, float depth = 0.f) const;
        /// Unproject screen-space coordinates to a forward-looking transform.
        lava::Transform unprojectAsTransform(const glm::vec2& coordinates, float depth = 0.f) const;
        /// Unproject screen-space rectangle to a 3D view frustum.
        magma::Frustum unprojectAsFrustum(const glm::vec2& topLeftCoordinates, const glm::vec2& bottomRightCoordinates) const;
        /// @}

    private:
        magma::Camera* m_camera = nullptr;
        magma::OrbitCameraController m_cameraController;

        Extent2d m_extent;
        uint32_t m_onWindowExtentChangedId;
    };
}
