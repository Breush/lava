#pragma once

#include <lava/magma/cameras/orbit-camera.hpp>

#include <lava/magma/render-engine.hpp>
#include <vulkan/vulkan.hpp>

#include "../holders/ubo-holder.hpp"

namespace lava::magma {
    /**
     * Implementation of lava::OrbitCamera.
     */
    class OrbitCamera::Impl {
    public:
        Impl(RenderEngine& engine);
        ~Impl();

        // ICamera
        ICamera::UserData render(ICamera::UserData data);
        const glm::vec3& position() const { return m_position; }
        const glm::mat4& viewTransform() const { return m_viewTransform; }
        const glm::mat4& projectionTransform() const { return m_projectionTransform; }

        // OrbitCamera
        void position(const glm::vec3& position);
        void target(const glm::vec3& target);
        void viewportRatio(float viewportRatio);

    protected:
        void updateBindings();
        void updateViewTransform();
        void updateProjectionTransform();

    private:
        // References
        RenderEngine::Impl& m_engine;

        // Descriptor
        vk::DescriptorSet m_descriptorSet;
        vulkan::UboHolder m_uboHolder;

        // Configuration
        $attribute(glm::vec3, target);
        $attribute(float, viewportRatio, = 16.f / 9.f);

        // Attributes
        glm::vec3 m_position;
        glm::mat4 m_viewTransform;
        glm::mat4 m_projectionTransform;
    };
}
