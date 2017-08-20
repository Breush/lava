#pragma once

#include "./render-stage.hpp"

#include <vector>

#include "../holders/descriptor-holder.hpp"
#include "../holders/image-holder.hpp"

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
        void imageView(const vk::ImageView& imageView, const vk::Sampler& sampler);

    protected:
        // RenderStage
        void stageInit() override final;
        void stageUpdate() override final;
        void stageRender(const vk::CommandBuffer& commandBuffer) override final;

        void createFramebuffers();

    private:
        // References
        const vulkan::SwapchainHolder* m_swapchainHolder = nullptr;

        // Resources
        vulkan::ShaderModule m_vertexShaderModule;
        vulkan::ShaderModule m_fragmentShaderModule;
        vulkan::DescriptorHolder m_descriptorHolder;
        vk::DescriptorSet m_descriptorSet;
        std::vector<vulkan::Framebuffer> m_framebuffers;
    };
}
