#pragma once

#include "./i-renderer-stage.hpp"

#include <lava/magma/scene.hpp>

#include <lava/magma/ubos.hpp>

#include "../holders/image-holder.hpp"
#include "../holders/pipeline-holder.hpp"
#include "../holders/render-pass-holder.hpp"

namespace lava::magma {
    /**
     * Forward renderer for flat objects.
     */
    class ForwardFlatStage final : public IRendererStage {
        constexpr static const uint32_t MATERIAL_DESCRIPTOR_SET_INDEX = 0u;
        constexpr static const uint32_t CAMERA_PUSH_CONSTANT_OFFSET = 0u;
        constexpr static const uint32_t FLAT_PUSH_CONSTANT_OFFSET = sizeof(CameraUbo);

    public:
        ForwardFlatStage(Scene& scene);

        // IRendererStage
        void init(const Camera& camera) final;
        void rebuild() final;
        void render(vk::CommandBuffer commandBuffer, uint32_t frameId) final;

        void extent(vk::Extent2D extent) final;
        void sampleCount(vk::SampleCountFlagBits /* sampleCount */) final { /* Not handled */ }
        void polygonMode(vk::PolygonMode /* polygonMode */) final { /* Not handled */ }

        RenderImage renderImage() const final;
        RenderImage depthRenderImage() const final;
        vk::RenderPass renderPass() const final { return m_renderPassHolder.renderPass(); }

        void changeRenderImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer) final;

    protected:
        void initPass();

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
        vk::Extent2D m_extent;

        // Pass and subpasses
        vulkan::RenderPassHolder m_renderPassHolder;
        vulkan::PipelineHolder m_pipelineHolder;

        // Resources
        vulkan::ImageHolder m_finalImageHolder;
        vulkan::Framebuffer m_framebuffer;
    };
}
