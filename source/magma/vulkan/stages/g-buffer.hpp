#pragma once

#include "./render-stage.hpp"

#include "../image-holder.hpp"

namespace lava::magma {
    /**
     * Pipeline layout for the G-Buffer construction.
     */
    class GBuffer final : public RenderStage {
    public:
        GBuffer(RenderEngine::Impl& engine);

        const vulkan::ImageView& normalImageView() const { return m_normalImageHolder.view(); }
        const vulkan::ImageView& albedoImageView() const { return m_albedoImageHolder.view(); }
        const vulkan::ImageView& ormImageView() const { return m_ormImageHolder.view(); }
        const vulkan::ImageView& depthImageView() const { return m_depthImageHolder.view(); }

    protected:
        // RenderStage
        void stageInit() override final;
        void stageUpdate() override final;
        void stageRender(const vk::CommandBuffer& commandBuffer) override final;

        vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo() override final;
        vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo() override final;

        void createResources();
        void createFramebuffers();

    private:
        // Resources
        vulkan::ShaderModule m_vertexShaderModule;
        vulkan::ShaderModule m_fragmentShaderModule;
        vulkan::ImageHolder m_normalImageHolder;
        vulkan::ImageHolder m_albedoImageHolder;
        vulkan::ImageHolder m_ormImageHolder;
        vulkan::ImageHolder m_depthImageHolder;
        vulkan::Framebuffer m_framebuffer;

        // Internal configuration
        vk::VertexInputBindingDescription m_vertexInputBindingDescription;
        std::vector<vk::VertexInputAttributeDescription> m_vertexInputAttributeDescriptions;
    };
}
