#pragma once

#include "./render-stage.hpp"

#include <lava/magma/render-scenes/render-scene.hpp>

#include "../holders/descriptor-holder.hpp"
#include "../holders/image-holder.hpp"

namespace lava::magma {
    /**
     * Pipeline layout for the G-Buffer construction.
     */
    class GBuffer final : public RenderStage {
    public:
        GBuffer(RenderScene::Impl& scene);

        vk::ImageView normalImageView() const { return m_normalImageHolder.view(); }
        vk::ImageView albedoImageView() const { return m_albedoImageHolder.view(); }
        vk::ImageView ormImageView() const { return m_ormImageHolder.view(); }
        vk::ImageView depthImageView() const { return m_depthImageHolder.view(); }

        const vulkan::DescriptorHolder& cameraDescriptorHolder() const { return m_cameraDescriptorHolder; }
        const vulkan::DescriptorHolder& materialDescriptorHolder() const { return m_materialDescriptorHolder; }
        const vulkan::DescriptorHolder& meshDescriptorHolder() const { return m_meshDescriptorHolder; }

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
        // References
        RenderScene::Impl& m_scene;

        // Resources
        vulkan::ShaderModule m_vertexShaderModule;
        vulkan::ShaderModule m_fragmentShaderModule;
        vulkan::ImageHolder m_normalImageHolder;
        vulkan::ImageHolder m_albedoImageHolder;
        vulkan::ImageHolder m_ormImageHolder;
        vulkan::ImageHolder m_depthImageHolder;
        vulkan::DescriptorHolder m_cameraDescriptorHolder;
        vulkan::DescriptorHolder m_materialDescriptorHolder;
        vulkan::DescriptorHolder m_meshDescriptorHolder;
        vulkan::Framebuffer m_framebuffer;

        // Internal configuration
        vk::VertexInputBindingDescription m_vertexInputBindingDescription;
        std::vector<vk::VertexInputAttributeDescription> m_vertexInputAttributeDescriptions;
    };
}
