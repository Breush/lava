#pragma once

#include "./i-renderer-stage.hpp"

#include <lava/magma/scene.hpp>

#include <lava/magma/ubos.hpp>

#include "../holders/image-holder.hpp"
#include "../holders/pipeline-holder.hpp"
#include "../holders/render-pass-holder.hpp"

namespace lava::magma {
    /**
     * Forward renderer.
     */
    class ForwardRendererStage final : public IRendererStage {
        constexpr static const uint32_t ENVIRONMENT_DESCRIPTOR_SET_INDEX = 0u;
        constexpr static const uint32_t MATERIAL_DESCRIPTOR_SET_INDEX = 1u;
        constexpr static const uint32_t MATERIAL_GLOBAL_DESCRIPTOR_SET_INDEX = 2u;
        constexpr static const uint32_t LIGHTS_DESCRIPTOR_SET_INDEX = 3u;
        constexpr static const uint32_t SHADOWS_DESCRIPTOR_SET_INDEX = 4u;
        constexpr static const uint32_t CAMERA_PUSH_CONSTANT_OFFSET = 0u;
        constexpr static const uint32_t MESH_PUSH_CONSTANT_OFFSET = sizeof(CameraUbo);

    public:
        ForwardRendererStage(Scene& scene);

        // IRendererStage
        void init(const Camera& camera) final;
        void rebuild() final;
        void record(vk::CommandBuffer commandBuffer, uint32_t frameId) final;

        void extent(vk::Extent2D extent) final;
        void sampleCount(vk::SampleCountFlagBits sampleCount) final;
        void polygonMode(vk::PolygonMode polygonMode) final;

        RenderImage renderImage() const final;
        RenderImage depthRenderImage() const final;
        bool depthRenderImageValid() const final { return !m_msaaEnabled; }
        vk::RenderPass renderPass() const final { return m_renderPassHolder.renderPass(); }

        void changeRenderImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer) final;

    protected:
        void initOpaquePass();
        void initDepthlessPass();
        void initWireframePass();
        void initTranslucentPass();

        void updatePassShaders(bool firstTime);

        void createResources();
        void createFramebuffers();

    private:
        // References
        Scene& m_scene;
        const Camera* m_camera = nullptr;

        bool m_rebuildRenderPass = true;
        bool m_rebuildPipelines = true;
        bool m_rebuildResources = true;

        // Configuration
        bool m_msaaEnabled = false;
        vk::Extent2D m_extent;
        vk::PolygonMode m_polygonMode = vk::PolygonMode::eFill;
        vk::SampleCountFlagBits m_sampleCount = vk::SampleCountFlagBits::e1;

        // Pass and subpasses
        vulkan::RenderPassHolder m_renderPassHolder;
        vulkan::PipelineHolder m_opaquePipelineHolder;
        vulkan::PipelineHolder m_depthlessPipelineHolder;
        vulkan::PipelineHolder m_wireframePipelineHolder;
        vulkan::PipelineHolder m_translucentPipelineHolder;

        // Resources
        vulkan::ImageHolder m_finalImageHolder;
        vulkan::ImageHolder m_finalResolveImageHolder;
        vulkan::ImageHolder m_depthImageHolder;
        vulkan::Framebuffer m_framebuffer;
    };
}
