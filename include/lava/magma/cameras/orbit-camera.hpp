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
        Extent2d extent() const final;
        void extent(Extent2d extent) final;
        RenderImage renderImage() const final;
        RenderImage depthRenderImage() const final;
        PolygonMode polygonMode() const final;
        void polygonMode(PolygonMode polygonMode) final;
        float nearClip() const final;
        void nearClip(float nearClip) final;
        float farClip() const final;
        void farClip(float farClip) final;
        const glm::mat4& viewTransform() const;
        const glm::mat4& projectionTransform() const;

        ICamera::Impl& interfaceImpl() final;
        const ICamera::Impl& interfaceImpl() const final;

        // Attributes
        const glm::vec3& translation() const;
        void translation(const glm::vec3& translation);

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
