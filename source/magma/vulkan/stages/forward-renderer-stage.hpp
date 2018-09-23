#pragma once

#include "./i-renderer-stage.hpp"

#include <lava/magma/render-scenes/render-scene.hpp>

#include "../holders/buffer-holder.hpp"
#include "../holders/descriptor-holder.hpp"
#include "../holders/image-holder.hpp"
#include "../holders/pipeline-holder.hpp"
#include "../holders/render-pass-holder.hpp"
#include "../holders/ubo-holder.hpp"

namespace lava::magma {
    /**
     * Forward renderer.
     */
    class ForwardRendererStage final : public IRendererStage {
        constexpr static const uint32_t CAMERA_DESCRIPTOR_SET_INDEX = 0u;
        constexpr static const uint32_t MATERIAL_DESCRIPTOR_SET_INDEX = 1u;
        constexpr static const uint32_t MESH_DESCRIPTOR_SET_INDEX = 2u;
        constexpr static const uint32_t LIGHTS_DESCRIPTOR_SET_INDEX = 3u;

    public:
        ForwardRendererStage(RenderScene::Impl& scene);

        // IRendererStage
        void init(uint32_t cameraId) override final;
        void update(vk::Extent2D extent, vk::PolygonMode polygonMode) override final;
        void render(vk::CommandBuffer commandBuffer) override final;

        RenderImage renderImage() const override final;
        RenderImage depthRenderImage() const override final;

    protected:
        void initOpaquePass();
        void initTranslucentPass();

        void updatePassShaders(bool firstTime);

        void createResources();
        void createFramebuffers();

    private:
        // References
        RenderScene::Impl& m_scene;
        uint32_t m_cameraId = -1u;
        vk::Extent2D m_extent;

        // Pass and subpasses
        vulkan::RenderPassHolder m_renderPassHolder;
        vulkan::PipelineHolder m_opaquePipelineHolder;
        vulkan::PipelineHolder m_translucentPipelineHolder;

        // Resources
        vulkan::ImageHolder m_finalImageHolder;
        vulkan::ImageHolder m_depthImageHolder;
        vulkan::Framebuffer m_framebuffer;
    };
}
