#pragma once

#include <lava/magma/cameras/i-camera.hpp>

#include <glm/glm.hpp>

namespace lava::magma {
    class RenderScene;
}

namespace lava::magma {
    /**
     * Orbiting camera.
     */
    class OrbitCamera final : public ICamera {
    public:
        OrbitCamera(RenderScene& scene, Extent2d extent);
        ~OrbitCamera();

        // ICamera
        Extent2d extent() const;
        void extent(Extent2d extent);

        ICamera::Impl& interfaceImpl();

        // Attributes
        const glm::vec3& position() const;
        void position(const glm::vec3& position);

        const glm::vec3& target() const;
        void target(const glm::vec3& target);

        float radius() const;
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
