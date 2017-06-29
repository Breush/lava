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

        // ICamera
        const glm::vec3& position() const { return m_position; }
        const glm::mat4& viewTransform() const { return m_viewTransform; }
        const glm::mat4& projectionTransform() const { return m_projectionTransform; }

        void position(const glm::vec3& position);
        const glm::vec3& target() const { return m_target; }
        void target(const glm::vec3& target);
        void viewportRatio(float viewportRatio);

    protected:
        void updateViewTransform();
        void updateProjectionTransform();

    private:
        // References
        RenderEngine::Impl& m_engine;

        // Attributes
        glm::mat4 m_viewTransform;
        glm::mat4 m_projectionTransform;
        glm::vec3 m_position;
        glm::vec3 m_target;
        float m_viewportRatio = 16.f / 9.f;
    };
}
