#pragma once

#include <lava/magma/interfaces/camera.hpp>

#include <glm/glm.hpp>

namespace lava {
    /**
     * Orbiting camera.
     */
    class OrbitCamera final : public ICamera {
    public:
        OrbitCamera(RenderEngine& engine);
        ~OrbitCamera();

        // ICamera
        const glm::vec3& position() const override final;
        const glm::mat4& viewTransform() const override final;
        const glm::mat4& projectionTransform() const override final;

        void position(const glm::vec3& position);
        void target(const glm::vec3& target);
        void viewportRatio(float viewportRatio);

        void latitudeAdd(float latitudeDelta);
        void longitudeAdd(float longitudeDelta);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
