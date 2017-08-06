#pragma once

#include <lava/magma/cameras/orbit-camera.hpp>

#include <lava/magma/render-engine.hpp>
#include <vulkan/vulkan.hpp>

#include "../buffer-holder.hpp"

namespace lava::magma {
    /**
     * Implementation of lava::OrbitCamera.
     */
    class OrbitCamera::Impl {
        // @todo This is all shader thing - Should it be in a ICamera::Impl?
        constexpr static const auto DESCRIPTOR_SET_INDEX = 0u;
        struct CameraUbo {
            glm::mat4 view;
            glm::mat4 projection;
        };

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
        const glm::vec3& target() const { return m_target; }
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
        vulkan::BufferHolder m_uniformBufferHolder;

        // Attributes
        glm::mat4 m_viewTransform;
        glm::mat4 m_projectionTransform;
        glm::vec3 m_position;
        glm::vec3 m_target;
        float m_viewportRatio = 16.f / 9.f;
    };
}
