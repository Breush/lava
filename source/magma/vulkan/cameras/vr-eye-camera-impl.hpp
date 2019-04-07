#pragma once

#include <lava/magma/cameras/vr-eye-camera.hpp>

#include "./i-camera-impl.hpp"

#include <lava/magma/render-scenes/render-scene.hpp>

#include "../holders/ubo-holder.hpp"

namespace lava::magma {
    /**
     * Implementation of lava::VrEyeCamera.
     */
    class VrEyeCamera::Impl final : public ICamera::Impl {
    public:
        Impl(RenderScene& scene, Extent2d extent);
        ~Impl();

        // ICamera
        Extent2d extent() const { return {m_extent.width, m_extent.height}; }
        void extent(Extent2d extent);
        RenderImage renderImage() const;
        RenderImage depthRenderImage() const;
        PolygonMode polygonMode() const { return m_polygonMode; }
        void polygonMode(PolygonMode polygonMode);

        // ICamera::Impl
        void init(uint32_t id) final;
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex) const final;
        vk::Extent2D renderExtent() const final { return m_extent; }
        const glm::vec3& translation() const final { return m_translation; }
        const glm::mat4& viewTransform() const final { return m_viewTransform; }
        const glm::mat4& projectionTransform() const final { return m_projectionTransform; }
        bool useFrustumCulling() const final { return false; } // @fixme Can't get that working
        const Frustum& frustum() const final { return m_frustum; }

        // Internal interface
        void update(vr::EVREye eye, const glm::mat4& hmdTransform);
        void forceProjectionTransform(const glm::mat4& projectionTransform);
        void forceViewTransform(glm::mat4 viewTransform);
        void changeImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer);

    protected:
        void updateFrustum();
        void updateBindings();

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
        PolygonMode m_polygonMode = PolygonMode::Fill;

        // Attributes
        glm::vec3 m_translation;
        glm::mat4 m_viewTransform;
        glm::mat4 m_projectionTransform;
        glm::mat4 m_fixesTransform;
        Frustum m_frustum;
    };
}
