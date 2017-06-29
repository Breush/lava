#pragma once

#include <lava/magma/cameras/orbit-camera.hpp>
#include <lava/magma/render-engine.hpp>

namespace lava {
    /**
     * Vulkan-based implementation of lava::OrbitCamera.
     */
    class OrbitCamera::Impl {
    public:
        Impl(RenderEngine& engine);
        ~Impl();

        void position(const glm::vec3& position);
        void target(const glm::vec3& target);

    private:
        // References
        RenderEngine::Impl& m_engine;

        // Attributes
        glm::vec3 m_position;
        glm::vec3 m_target;
    };
}
