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
        Impl(RenderScene& scene, Extent2d extent);
        ~Impl();

        // ICamera
        Extent2d extent() const { return {m_extent.width, m_extent.height}; }
        void extent(Extent2d extent);
        RenderImage renderImage() const;
        RenderImage depthRenderImage() const;

        // ICamera::Impl
        void init(uint32_t id) override final;
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                    uint32_t descriptorSetIndex) const override final;
        vk::Extent2D renderExtent() const override final { return m_extent; }
        const glm::vec3& position() const override final { return m_position; }
        const glm::mat4& viewTransform() const override final { return m_viewTransform; }
        const glm::mat4& projectionTransform() const override final { return m_projectionTransform; }

        // OrbitCamera
        void position(const glm::vec3& position);
        void target(const glm::vec3& target);

    protected:
        void updateBindings();
        void updateViewTransform();
        void updateProjectionTransform();

    private:
        // References
        RenderScene::Impl& m_scene;
        uint32_t m_id = -1u;
        bool m_initialized = false;

        // Descriptor
        vk::DescriptorSet m_descriptorSet;
        vulkan::UboHolder m_uboHolder;

        // Configuration
        vk::Extent2D m_extent;
        $attribute(glm::vec3, target);

        // Attributes
        glm::vec3 m_position;
        glm::mat4 m_viewTransform;
        glm::mat4 m_projectionTransform;
    };
}
