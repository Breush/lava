#pragma once

#include "./i-stage.hpp"

#include "../image-holder.hpp"

namespace lava::magma {
    /**
     * Pipeline layout for the G-Buffer construction.
     */
    class GBuffer final : public IStage {
    public:
        GBuffer(RenderEngine::Impl& engine);

        // IStage
        void init() override final;
        void update(const vk::Extent2D& extent) override final;
        void render(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex) override final;

        const vulkan::ImageView& normalImageView() const { return m_normalImageHolder.view(); }
        const vulkan::ImageView& albedoImageView() const { return m_albedoImageHolder.view(); }
        const vulkan::ImageView& ormImageView() const { return m_ormImageHolder.view(); }
        const vulkan::ImageView& depthImageView() const { return m_depthImageHolder.view(); }

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
        vulkan::ImageHolder m_normalImageHolder;
        vulkan::ImageHolder m_albedoImageHolder;
        vulkan::ImageHolder m_ormImageHolder;
        vulkan::ImageHolder m_depthImageHolder;
        vulkan::Framebuffer m_framebuffer;
    };
}
