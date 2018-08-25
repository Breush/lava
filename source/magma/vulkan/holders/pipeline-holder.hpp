#pragma once

#include <lava/magma/render-engine.hpp>

#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    /**
     * Holds a vulkan::Pipeline.
     */
    class PipelineHolder final {
    public:
        enum class ColorAttachmentBlending {
            None,
            AlphaBlending,
        };

        struct ColorAttachment {
            vk::Format format;
            vk::ImageLayout finalLayout;
            ColorAttachmentBlending blending = ColorAttachmentBlending::None;
        };

        struct DepthStencilAttachment {
            vk::Format format;
        };

        struct InputAttachment {
            vk::Format format;
        };

        struct VertexInput {
            struct Attribute {
                vk::Format format;
                uint32_t offset;
            };

            uint32_t stride;
            std::vector<Attribute> attributes;
        };

    public:
        PipelineHolder(RenderEngine::Impl& engine);

        void init(uint32_t subpassIndex);
        void update(vk::Extent2D extent);

        const vulkan::Pipeline& pipeline() const { return m_pipeline; }
        const vulkan::PipelineLayout& pipelineLayout() const { return m_pipelineLayout; }

        /// Register a descriptor set layout. Order is important.
        void add(const vk::DescriptorSetLayout& descriptorSetLayout);

        /// Register a shader stage.
        void add(const vk::PipelineShaderStageCreateInfo& shaderStage);
        void removeShaderStages() { m_shaderStages.clear(); }

        /// Register a color attachment.
        void add(const ColorAttachment& colorAttachment);

        /// Set the depth/stencil attachment.
        void set(const DepthStencilAttachment& depthStencilAttachment);

        /// Register an input attachment.
        void add(const InputAttachment& inputAttachment);

        /// Set the vertex input.
        void set(const VertexInput& vertexInput) { m_vertexInput = vertexInput; }

        /// Set the cull mode.
        void set(vk::CullModeFlags cullMode) { m_cullMode = cullMode; }

        /**
         * Whether the pipeline is for a self-dependent pass.
         * This is used when then pass needs a pipeline barrier
         * while being bound. For example, between to mesh rendering.
         */
        void selfDependent(bool selfDependent) { m_selfDependent = selfDependent; }

        //----- Getters

        const std::vector<ColorAttachment>& colorAttachments() const { return m_colorAttachments; }
        const std::unique_ptr<DepthStencilAttachment>& depthStencilAttachment() const { return m_depthStencilAttachment; }
        const std::vector<InputAttachment>& inputAttachments() const { return m_inputAttachments; }
        bool selfDependent() const { return m_selfDependent; }

        // Internal API
        void bindRenderPass(const vk::RenderPass& renderPass) { m_renderPass = &renderPass; }

    protected:
        void initPipelineLayout();

    private:
        // References
        RenderEngine::Impl& m_engine;
        const vk::RenderPass* m_renderPass = nullptr;
        uint32_t m_subpassIndex = 0u;

        // Render pipeline
        vulkan::PipelineLayout m_pipelineLayout;
        vulkan::Pipeline m_pipeline;

        // Internals
        std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
        std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStages;
        std::vector<ColorAttachment> m_colorAttachments;
        std::unique_ptr<DepthStencilAttachment> m_depthStencilAttachment;
        std::vector<InputAttachment> m_inputAttachments;
        VertexInput m_vertexInput;
        vk::CullModeFlags m_cullMode;
        bool m_selfDependent = false;
    };
}
