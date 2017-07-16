#pragma once

#include "./i-stage.hpp"

#include "../image-holder.hpp"

namespace lava::magma {
    /**
     * Pipeline layout for the final step of presenting to the screen.
     */
    class Epiphany final : public IStage {
    public:
        Epiphany(RenderEngine::Impl& engine);

        // IStage
        void init() override final;
        void update() override final;
        void render(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex) override final;

        void normalImageView(const vk::ImageView& imageView, const vk::Sampler& sampler);
        void albedoImageView(const vk::ImageView& imageView, const vk::Sampler& sampler);
        void ormImageView(const vk::ImageView& imageView, const vk::Sampler& sampler);
        void depthImageView(const vk::ImageView& imageView, const vk::Sampler& sampler);
        const vulkan::ImageView& imageView() const { return m_imageHolder.view(); }

    protected:
        void createRenderPass();
        void createGraphicsPipeline();
        void createResources();
        void createFramebuffers();

        void updateUbos();

    private:
        // Resources
        vulkan::ShaderModule m_vertShaderModule;
        vulkan::ShaderModule m_fragShaderModule;
        vulkan::DescriptorPool m_descriptorPool;
        vulkan::DescriptorSetLayout m_descriptorSetLayout;
        vk::DescriptorSet m_descriptorSet;
        vulkan::ImageHolder m_imageHolder;
        vulkan::Framebuffer m_framebuffer;

        // @todo Make BufferHolder (Uniform only?)
        vulkan::Buffer m_cameraUniformStagingBuffer;
        vulkan::DeviceMemory m_cameraUniformStagingBufferMemory;
        vulkan::Buffer m_cameraUniformBuffer;
        vulkan::DeviceMemory m_cameraUniformBufferMemory;

        vulkan::Buffer m_lightUniformStagingBuffer;
        vulkan::DeviceMemory m_lightUniformStagingBufferMemory;
        vulkan::Buffer m_lightUniformBuffer;
        vulkan::DeviceMemory m_lightUniformBufferMemory;
    };
}
