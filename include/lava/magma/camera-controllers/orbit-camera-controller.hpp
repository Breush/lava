#pragma once

#include <glm/glm.hpp>

namespace lava::magma {
    class Camera;
}

namespace lava::magma {
    /**
     * Orbiting camera controls.
     * Always points to a target point and rotates around.
     */
    class OrbitCameraController {
    public:
        OrbitCameraController() {}
        OrbitCameraController(Camera& camera) { bind(camera); }

        /// Bind a camera to this controller.
        void bind(Camera& camera);

        /// To be called when the binded camera has an init-time configuration changed.
        void updateCamera();

        /**
         * @name Controls
         */
        /// @{
        /// Which vector is considered up.
        const glm::vec3& up() const { return m_up; }
        void up(const glm::vec3& up);

        /// Move the camera at a certain position. Keeps target, changes radius.
        const glm::vec3& origin() const { return m_origin; }
        void origin(const glm::vec3& origin);

        /// Target a certain point. Keeps origin, changes radius.
        const glm::vec3& target() const { return m_target; }
        void target(const glm::vec3& target);

        /// Change radius. Keeps target, changes origin.
        float radius() const { return glm::length(m_target - m_origin); }
        void radius(float radius);
        void radiusAdd(float delta) { radius(radius() + delta); }

        /// Move according to left and up of the camera. Keeps radius, changes origin and target.
        void strafe(float x, float y);

        /// Orbit around the target point by a certain delta. Keeps radius and target, changes origin.
        void orbitAdd(float longitudeAngle, float latitudeAngle);
        /// @}

    protected:
        void updateCameraViewTransform();
        void updateCameraProjectionTransform();

    private:
        Camera* m_camera;

        // ----- Controls
        glm::vec3 m_up = glm::vec3(0.f, 0.f, 1.f);
        glm::vec3 m_origin = glm::vec3(-1.f, 0.f, 0.f);
        glm::vec3 m_target = glm::vec3(0.f);
    };
}
