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
        constexpr static const uint32_t CAMERA_DESCRIPTOR_SET_INDEX = 0u;
        constexpr static const uint32_t MATERIAL_DESCRIPTOR_SET_INDEX = 1u;
        constexpr static const uint32_t MESH_DESCRIPTOR_SET_INDEX = 2u;

    public:
        GBuffer(RenderScene::Impl& scene);

        void init(uint32_t cameraId);

        vk::ImageView normalImageView() const { return m_normalImageHolder.view(); }
        vk::ImageView albedoImageView() const { return m_albedoImageHolder.view(); }
        vk::ImageView ormImageView() const { return m_ormImageHolder.view(); }
        vk::ImageView depthImageView() const { return m_depthImageHolder.view(); }

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
        uint32_t m_cameraId = -1u;

        // Resources
        vk::ShaderModule m_vertexShaderModule = nullptr;
        vk::ShaderModule m_fragmentShaderModule = nullptr;
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
