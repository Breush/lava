#pragma once

#include <lava/magma/render-engine.hpp>
#include <vulkan/vulkan.hpp>

#include "../capsule.hpp"
#include "../wrappers.hpp"

namespace lava::magma {
    /**
     * Pipeline layout for the G-Buffer construction.
     */
    class GBuffer final {
    public:
        GBuffer(RenderEngine::Impl& engine);

        inline const vk::RenderPass& renderPass() const { return m_renderPass; }
        inline const vk::PipelineLayout& pipelineLayout() const { return m_pipelineLayout; }

        void beginRender(const vk::CommandBuffer& commandBuffer, const vk::Framebuffer& framebuffer);
        void endRender(const vk::CommandBuffer& commandBuffer);

        void createRenderPass();
        void createGraphicsPipeline();

    private:
        RenderEngine::Impl& m_engine;

        // Graphics pipeline
        // @todo We should have our own framebuffers
        vulkan::RenderPass m_renderPass;
        vulkan::PipelineLayout m_pipelineLayout;
        vulkan::Pipeline m_graphicsPipeline;
    };
}
