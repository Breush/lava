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

        void position(const glm::vec3& position);
        void target(const glm::vec3& target);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
