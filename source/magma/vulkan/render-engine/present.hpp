#pragma once

#include "./i-stage.hpp"

#include <vector>

#include "../image-holder.hpp"

namespace lava::magma {
    /**
     * Pipeline layout for the final step of presenting to the screen.
     */
    class Present final : public IStage {
    public:
        Present(RenderEngine::Impl& engine);

        // IStage
        void init() override final;
        void update(const vk::Extent2D& extent) override final;
        void render(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex) override final;

        void shownImageView(const vk::ImageView& imageView, const vk::Sampler& sampler);

    protected:
        void createRenderPass();
        void createGraphicsPipeline();
        void createResources();
        void createFramebuffers();

    private:
        vk::Extent2D m_extent;

        // Resources
        vulkan::ShaderModule m_vertShaderModule;
        vulkan::ShaderModule m_fragShaderModule;
        vulkan::DescriptorPool m_descriptorPool;
        vulkan::DescriptorSetLayout m_descriptorSetLayout;
        vk::DescriptorSet m_descriptorSet;
        std::vector<vulkan::Framebuffer> m_framebuffers;
    };
}
