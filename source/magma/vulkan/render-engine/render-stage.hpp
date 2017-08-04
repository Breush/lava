#pragma once

#include "./i-stage.hpp"

#include <lava/magma/render-engine.hpp>

#include "../wrappers.hpp"

namespace lava::magma {
    class RenderStage : public IStage {
    public:
        RenderStage(RenderEngine::Impl& engine);
        ~RenderStage() = default;

        // IStage
        void init() override final;
        void update(const vk::Extent2D& extent) override final;
        void render(const vk::CommandBuffer& commandBuffer) override final;

    protected:
        //----- Subclass API

        /// Called once in a runtime.
        virtual void stageInit() = 0;

        /// Called each time the extent changes.
        virtual void stageUpdate() = 0;

        /// Called each frame.
        virtual void stageRender(const vk::CommandBuffer& commandBuffer) = 0;

        //----- Configuration structures

        struct ColorAttachment {
            vk::Format format;
            vk::ImageLayout finalLayout;
        };

        struct DepthStencilAttachment {
            vk::Format format;
        };

        //----- Internals API

        /// Skipping updates that use the same extent as current. Default is to skip, but this should be disabled if one intends
        /// to use any swapchain information.
        void updateCleverlySkipped(bool updateCleverlySkipped) { m_updateCleverlySkipped = updateCleverlySkipped; }

        /// Register a descriptor set layout. Order is important.
        void add(const vk::DescriptorSetLayout& descriptorSetLayout);

        /// Register a shader stage.
        void add(const vk::PipelineShaderStageCreateInfo& shaderStage);

        /// Register a color attachment.
        void add(const ColorAttachment& colorAttachment);

        /// Set the depth/stencil attachment.
        void set(const DepthStencilAttachment& depthStencilAttachment);

    private:
        //----- Set-up

        /// Creates the render pass. All attachments must have been added.
        void initRenderPass();

        /// Creates the pipeline layout. All descriptors sets must have been added.
        void initPipelineLayout();

        // @todo The update could call that automatically.
        /// Creates the pipeline. All shader stages must have been added.
        void updatePipeline();

        //----- Pipeline creation defaults

        virtual vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo();
        virtual vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo();
        virtual vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo();
        virtual vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo();
        virtual vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo();
        virtual vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo();
        virtual vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo();

    protected:
        RenderEngine::Impl& m_engine;

        // Configuration
        vk::Extent2D m_extent;
        bool m_updateCleverlySkipped = true;

        // Render pipeline
        vulkan::RenderPass m_renderPass;
        vulkan::PipelineLayout m_pipelineLayout;
        vulkan::Pipeline m_pipeline;

    private:
        // Configuration internals
        std::vector<ColorAttachment> m_colorAttachments;
        std::unique_ptr<DepthStencilAttachment> m_depthStencilAttachment;
        std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
        std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStages;
        std::vector<vk::PipelineColorBlendAttachmentState> m_colorBlendAttachmentStates;
        vk::Rect2D m_scissor;
        vk::Viewport m_viewport;
    };
}
