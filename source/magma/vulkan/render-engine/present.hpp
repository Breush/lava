#pragma once

#include <lava/magma/render-engine.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "../image-holder.hpp"
#include "../wrappers.hpp"

namespace lava::magma {
    /**
     * Pipeline layout for the final step of presenting to the screen.
     */
    class Present {
    public:
        Present(RenderEngine::Impl& engine);

        void render(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex);

        void createRenderPass();
        void createGraphicsPipeline();
        void createResources();
        void createFramebuffers();

    private:
        RenderEngine::Impl& m_engine;

        // Render pipeline
        vulkan::RenderPass m_renderPass;
        vulkan::PipelineLayout m_pipelineLayout;
        vulkan::Pipeline m_pipeline;

        // Resources
        vulkan::Buffer m_vertexBuffer;
        vulkan::DeviceMemory m_vertexBufferMemory;
        std::vector<vulkan::Framebuffer> m_framebuffers;
    };
}
