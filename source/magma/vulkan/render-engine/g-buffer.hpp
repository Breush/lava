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

        // @todo Useless index once only one framebuffer (= no more presenting to swapchain in this pass)
        void beginRender(const vk::CommandBuffer& commandBuffer, uint32_t index);
        void endRender(const vk::CommandBuffer& commandBuffer);

        void createRenderPass();
        void createGraphicsPipeline();
        void createResources();
        void createFramebuffers();

    private:
        RenderEngine::Impl& m_engine;

        // Render pipeline
        vulkan::RenderPass m_renderPass;
        vulkan::PipelineLayout m_pipelineLayout;
        vulkan::Pipeline m_graphicsPipeline;

        // Resources
        // @todo Normal image view - currently using swapchain image views
        vulkan::ImageHolder m_albedoImageHolder;
        vulkan::ImageHolder m_depthImageHolder;
        std::vector<vulkan::Framebuffer> m_framebuffers;
    };
}
