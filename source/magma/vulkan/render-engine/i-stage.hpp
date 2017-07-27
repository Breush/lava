#pragma once

#include <lava/magma/render-engine.hpp>

#include "../wrappers.hpp"

namespace lava::magma {
    class IStage {
    public:
        IStage(RenderEngine::Impl& engine);

        /// Called once in a runtime.
        virtual void init() = 0;

        /// Called each time the extent changes.
        virtual void update(const vk::Extent2D& extent);

        /// Called each frame.
        virtual void render(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex) = 0;

    protected:
        // @todo The update could call that automatically.
        /// Creates the pipeline.
        void updatePipeline();

        //----- Pipeline creation defaults

        virtual std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos();
        virtual vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo();
        virtual vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo();
        virtual vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo();
        virtual vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo();
        virtual vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo();
        virtual vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo();

    protected:
        RenderEngine::Impl& m_engine;

        // Configuration
        vk::Extent2D m_extent;
        vulkan::ShaderModule m_vertexShaderModule;
        vulkan::ShaderModule m_fragmentShaderModule;

        // Render pipeline
        vulkan::RenderPass m_renderPass;
        vulkan::PipelineLayout m_pipelineLayout;
        vulkan::Pipeline m_pipeline;

    private:
        // Configuration internals
        std::vector<vk::PipelineColorBlendAttachmentState> m_colorBlendAttachmentStates;
        vk::Rect2D m_scissor;
        vk::Viewport m_viewport;
    };
}
