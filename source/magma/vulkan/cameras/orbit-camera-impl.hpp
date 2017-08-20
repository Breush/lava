#pragma once

#include <lava/magma/cameras/orbit-camera.hpp>

#include "./i-camera-impl.hpp"

#include <lava/magma/render-scenes/render-scene.hpp>
#include <vulkan/vulkan.hpp>

#include "../holders/ubo-holder.hpp"

namespace lava::magma {
    /**
     * Implementation of lava::OrbitCamera.
     */
    class OrbitCamera::Impl final : public ICamera::Impl {
    public:
        Impl(RenderScene& scene);
        ~Impl();

        // ICamera::Impl
        void init() override final;
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                    uint32_t descriptorSetIndex) override final;
        const glm::vec3& position() const override final { return m_position; }
        const glm::mat4& viewTransform() const override final { return m_viewTransform; }
        const glm::mat4& projectionTransform() const override final { return m_projectionTransform; }

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
        RenderScene::Impl& m_scene;
        bool m_initialized = false;

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
