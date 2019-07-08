#pragma once

#include "./i-renderer-stage.hpp"

#include <lava/magma/render-scenes/render-scene.hpp>

#include "../holders/buffer-holder.hpp"
#include "../holders/descriptor-holder.hpp"
#include "../holders/image-holder.hpp"
#include "../holders/pipeline-holder.hpp"
#include "../holders/render-pass-holder.hpp"
#include "../ubos.hpp"

namespace lava::magma {
    /**
     * Forward renderer.
     */
    class ForwardRendererStage final : public IRendererStage {
        constexpr static const uint32_t ENVIRONMENT_DESCRIPTOR_SET_INDEX = 0u;
        constexpr static const uint32_t MATERIAL_DESCRIPTOR_SET_INDEX = 1u;
        constexpr static const uint32_t LIGHTS_DESCRIPTOR_SET_INDEX = 2u;
        constexpr static const uint32_t SHADOWS_DESCRIPTOR_SET_INDEX = 3u;
        constexpr static const uint32_t CAMERA_PUSH_CONSTANT_OFFSET = 0u;
        constexpr static const uint32_t MESH_PUSH_CONSTANT_OFFSET = sizeof(vulkan::CameraUbo);

    public:
        ForwardRendererStage(RenderScene::Impl& scene);

        // IRendererStage
        void init(uint32_t cameraId) final;
        void update(vk::Extent2D extent, vk::PolygonMode polygonMode) final;
        void render(vk::CommandBuffer commandBuffer, uint32_t frameId) final;

        RenderImage renderImage() const final;
        RenderImage depthRenderImage() const final;
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
        RenderScene::Impl& m_scene;
        uint32_t m_cameraId = -1u;
        vk::Extent2D m_extent;
        vk::PolygonMode m_polygonMode = vk::PolygonMode::eFill;

        // Pass and subpasses
        vulkan::RenderPassHolder m_renderPassHolder;
        vulkan::PipelineHolder m_opaquePipelineHolder;
        vulkan::PipelineHolder m_depthlessPipelineHolder;
        vulkan::PipelineHolder m_wireframePipelineHolder;
        vulkan::PipelineHolder m_translucentPipelineHolder;

        // Resources
        vulkan::ImageHolder m_finalImageHolder;
        vulkan::ImageHolder m_depthImageHolder;
        vulkan::Framebuffer m_framebuffer;
    };
}
