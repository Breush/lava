#pragma once

#include "../holders/descriptor-holder.hpp"
#include "../holders/image-holder.hpp"
#include "../holders/pipeline-holder.hpp"
#include "../holders/render-pass-holder.hpp"
#include "../holders/ubo-holder.hpp"

#include <lava/magma/ubos.hpp>

namespace lava::magma::vulkan {
    class SwapchainHolder;
}

namespace lava::magma {
    /**
     * Pipeline layout for the final step of presenting to the screen.
     */
    class Present final {
    public:
        Present(RenderEngine::Impl& engine);

        void init();
        void update(vk::Extent2D extent);
        void render(vk::CommandBuffer commandBuffer);

        void bindSwapchainHolder(const vulkan::SwapchainHolder& swapchainHolder);

        /// Render an image to binded swapchain in a specific viewport.
        uint32_t addView(vk::ImageView imageView, vk::ImageLayout imageLayout, vk::Sampler sampler, Viewport viewport, uint32_t channelCount);

        /// Remove the specified view.
        void removeView(uint32_t viewId);

        /// Update the image of the specified view.
        void updateView(uint32_t viewId, vk::ImageView imageView, vk::ImageLayout imageLayout, vk::Sampler sampler);

    protected:
        void createFramebuffers();
        void updateUbos();

    protected:
        struct ViewInfo {
            uint32_t id;
            Viewport viewport;
            vk::ImageView imageView;
            vk::ImageLayout imageLayout;
            vk::Sampler sampler;
            uint32_t channelCount;
        };

    private:
        // References
        RenderEngine::Impl& m_engine;
        const vulkan::SwapchainHolder* m_swapchainHolder = nullptr;
        vk::Extent2D m_extent;

        // Configuration
        std::unordered_map<uint32_t, ViewInfo> m_viewInfos; // All known viewports, key is viewId.

        // Resources
        vulkan::RenderPassHolder m_renderPassHolder;
        vulkan::PipelineHolder m_pipelineHolder;
        vulkan::DescriptorHolder m_descriptorHolder;
        vulkan::UboHolder m_uboHolder;
        vk::DescriptorSet m_descriptorSet;
        std::vector<vulkan::Framebuffer> m_framebuffers;

        uint32_t m_nextViewId = 0u;
    };
}
