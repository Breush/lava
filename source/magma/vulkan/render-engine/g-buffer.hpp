#pragma once

#include <lava/magma/render-engine.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "../image-holder.hpp"
#include "../wrappers.hpp"

namespace lava::magma {
    /**
     * Pipeline layout for the G-Buffer construction.
     */
    class GBuffer {
    public:
        GBuffer(RenderEngine::Impl& engine);

        inline const vk::PipelineLayout& pipelineLayout() const { return m_pipelineLayout; }

        void beginRender(const vk::CommandBuffer& commandBuffer);
        void endRender(const vk::CommandBuffer& commandBuffer);

        void createRenderPass();
        void createGraphicsPipeline();
        void createResources();
        void createFramebuffers();

        const vulkan::ImageView& normalImageView() const { return m_normalImageHolder.view(); }
        const vulkan::ImageView& albedoImageView() const { return m_albedoImageHolder.view(); }
        const vulkan::ImageView& depthImageView() const { return m_depthImageHolder.view(); }

    private:
        RenderEngine::Impl& m_engine;

        // Render pipeline
        vulkan::RenderPass m_renderPass;
        vulkan::PipelineLayout m_pipelineLayout;
        vulkan::Pipeline m_pipeline;

        // Resources
        vulkan::ImageHolder m_normalImageHolder;
        vulkan::ImageHolder m_albedoImageHolder;
        vulkan::ImageHolder m_depthImageHolder;
        vulkan::Framebuffer m_framebuffer;
    };
}
