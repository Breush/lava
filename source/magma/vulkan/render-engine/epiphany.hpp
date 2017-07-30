#pragma once

#include "./render-stage.hpp"

#include "../buffer-holder.hpp"
#include "../image-holder.hpp"

namespace lava::magma {
    /**
     * Pipeline layout for the final step of presenting to the screen.
     */
    class Epiphany final : public RenderStage {
    public:
        Epiphany(RenderEngine::Impl& engine);

        void normalImageView(const vk::ImageView& imageView, const vk::Sampler& sampler);
        void albedoImageView(const vk::ImageView& imageView, const vk::Sampler& sampler);
        void ormImageView(const vk::ImageView& imageView, const vk::Sampler& sampler);
        void depthImageView(const vk::ImageView& imageView, const vk::Sampler& sampler);
        const vulkan::ImageView& imageView() const { return m_imageHolder.view(); }

    protected:
        // RenderStage
        void stageInit() override final;
        void stageUpdate() override final;
        void stageRender(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex) override final;

        void createResources();
        void createFramebuffers();

        void updateUbos();

        // Light linked list
        void fillLll();

    private:
        // Resources
        vulkan::ShaderModule m_vertexShaderModule;
        vulkan::ShaderModule m_fragmentShaderModule;
        vulkan::DescriptorPool m_descriptorPool;
        vulkan::DescriptorSetLayout m_descriptorSetLayout;
        vk::DescriptorSet m_descriptorSet;
        vulkan::ImageHolder m_imageHolder;
        vulkan::BufferHolder m_cameraBufferHolder;
        vulkan::BufferHolder m_lightBufferHolder;
        vulkan::Framebuffer m_framebuffer;
    };
}
