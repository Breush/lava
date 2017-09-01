#pragma once

#include "./render-stage.hpp"

#include <vector>

#include "../holders/descriptor-holder.hpp"
#include "../holders/image-holder.hpp"
#include "../holders/ubo-holder.hpp"

namespace lava::magma::vulkan {
    class SwapchainHolder;
}

namespace lava::magma {
    /**
     * Pipeline layout for the final step of presenting to the screen.
     */
    class Present final : public RenderStage {
    public:
        Present(RenderEngine::Impl& engine);

        void bindSwapchainHolder(const vulkan::SwapchainHolder& swapchainHolder);

        /// Render an image to binded swapchain in a specific viewport.
        uint32_t addView(vk::ImageView imageView, vk::Sampler sampler, Viewport viewport);

        /// Update the image of the specified view.
        void updateView(uint32_t viewId, vk::ImageView imageView, vk::Sampler sampler);

    protected:
        // RenderStage
        void stageInit() override final;
        void stageUpdate() override final;
        void stageRender(const vk::CommandBuffer& commandBuffer) override final;

        void createFramebuffers();

    private:
        // References
        const vulkan::SwapchainHolder* m_swapchainHolder = nullptr;

        // Configuration
        std::vector<Viewport> m_viewports;

        // Resources
        vulkan::ShaderModule m_vertexShaderModule;
        vulkan::ShaderModule m_fragmentShaderModule;
        vulkan::DescriptorHolder m_descriptorHolder;
        vulkan::UboHolder m_uboHolder;
        vk::DescriptorSet m_descriptorSet;
        std::vector<vulkan::Framebuffer> m_framebuffers;
    };
}
