#pragma once

#include <lava/magma/interfaces/camera.hpp>

#include <glm/glm.hpp>

namespace lava::magma {
    /**
     * Orbiting camera.
     */
    class OrbitCamera final : public ICamera {
    public:
        OrbitCamera(RenderEngine& engine);
        ~OrbitCamera();

        // ICamera
        UserData render(UserData data) override final;

        /**
         * Transforms.
         */
        const glm::mat4& viewTransform() const;
        const glm::mat4& projectionTransform() const;

        const glm::vec3& position() const;
        void position(const glm::vec3& position);
        void target(const glm::vec3& target);
        void viewportRatio(float viewportRatio);

        void radius(float radius);

        /**
         * Relative motion.
         */
        void strafe(float x, float y);
        void radiusAdd(float radiusDistance);
        void orbitAdd(float longitudeAngle, float latitudeAngle);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
