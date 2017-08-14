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
        const glm::vec3& position() const override final;
        const glm::mat4& viewTransform() const override final;
        const glm::mat4& projectionTransform() const override final;

        // Control
        void position(const glm::vec3& position);

        const glm::vec3& target() const;
        void target(const glm::vec3& target);
        float viewportRatio() const;
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
