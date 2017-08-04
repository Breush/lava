#pragma once

#include "./render-stage.hpp"

#include <vector>

#include "../image-holder.hpp"

namespace lava::magma::vulkan {
    class Swapchain;
}

namespace lava::magma {
    /**
     * Pipeline layout for the final step of presenting to the screen.
     */
    class Present final : public RenderStage {
    public:
        Present(RenderEngine::Impl& engine);

        void bindSwapchain(vulkan::Swapchain& swapchain);
        void imageView(const vk::ImageView& imageView, const vk::Sampler& sampler);

    protected:
        // RenderStage
        void stageInit() override final;
        void stageUpdate() override final;
        void stageRender(const vk::CommandBuffer& commandBuffer) override final;

        void createResources();
        void createFramebuffers();

    private:
        // References
        vulkan::Swapchain* m_swapchain = nullptr;

        // Resources
        vulkan::ShaderModule m_vertexShaderModule;
        vulkan::ShaderModule m_fragmentShaderModule;
        vulkan::DescriptorPool m_descriptorPool;
        vulkan::DescriptorSetLayout m_descriptorSetLayout;
        vk::DescriptorSet m_descriptorSet;
        std::vector<vulkan::Framebuffer> m_framebuffers;
    };
}
