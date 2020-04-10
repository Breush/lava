#pragma once

#include "./i-renderer-stage.hpp"

#include <lava/magma/ubos.hpp>

#include "../../g-buffer-data.hpp"
#include "../holders/buffer-holder.hpp"
#include "../holders/descriptor-holder.hpp"
#include "../holders/image-holder.hpp"
#include "../holders/pipeline-holder.hpp"
#include "../holders/render-pass-holder.hpp"

namespace lava::magma {
    class Scene;
}

namespace lava::magma {
    /**
     * Deep deferred renderer.
     *
     * Constructs a deeply-linked-list for its GBuffer.
     *
     * @note Does not handle MSAA.
     */
    class DeepDeferredStage final : public IRendererStage {
        constexpr static const uint32_t DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH = 3u;
        constexpr static const uint32_t DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT =
            4u; // Enough to store (G_BUFFER_DATA_SIZE + 2u)

        constexpr static const uint32_t DEEP_DEFERRED_GBUFFER_INPUT_DESCRIPTOR_SET_INDEX = 0u;
        constexpr static const uint32_t DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX = 1u;

        constexpr static const uint32_t GEOMETRY_MATERIAL_DESCRIPTOR_SET_INDEX = 2u;
        constexpr static const uint32_t GEOMETRY_MATERIAL_GLOBAL_DESCRIPTOR_SET_INDEX = 3u;

        constexpr static const uint32_t EPIPHANY_ENVIRONMENT_DESCRIPTOR_SET_INDEX = 2u;
        constexpr static const uint32_t EPIPHANY_LIGHTS_DESCRIPTOR_SET_INDEX = 3u;
        constexpr static const uint32_t EPIPHANY_SHADOWS_DESCRIPTOR_SET_INDEX = 4u;

        constexpr static const uint32_t CAMERA_PUSH_CONSTANT_OFFSET = 0u;
        constexpr static const uint32_t GEOMETRY_MESH_PUSH_CONSTANT_OFFSET = sizeof(CameraUbo);

        struct GBufferNode {
            // 26 bits can handle 8K resolution
            // 6 bits allows 64 different material shaders
            uint32_t materialId6_next26;
            float depth;
            uint32_t data[G_BUFFER_DATA_SIZE];
        };

    public:
        DeepDeferredStage(Scene& scene);

        // IRendererStage
        void init(const Camera& camera) final;
        void rebuild() final;
        void record(vk::CommandBuffer commandBuffer, uint32_t frameId) final;

        void extent(vk::Extent2D extent) final;
        void sampleCount(vk::SampleCountFlagBits /* sampleCount */) final { /* Not handled */ }
        void polygonMode(vk::PolygonMode polygonMode) final;

        RenderImage renderImage() const final;
        RenderImage depthRenderImage() const final;
        bool depthRenderImageValid() const final { return true; }
        vk::RenderPass renderPass() const final { return m_renderPassHolder.renderPass(); }

        void changeRenderImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer) final;

    protected:
        void initGBuffer();
        void initClearPass();
        void initGeometryPass();
        void initDepthlessPass();
        void initEpiphanyPass();

        void updateGeometryPassShaders(bool firstTime);
        void updateEpiphanyPassShaders(bool firstTime);

        void createResources();
        void createFramebuffers();

    private:
        // References
        Scene& m_scene;
        const Camera* m_camera = nullptr;

        bool m_rebuildRenderPass = true;
        bool m_rebuildPipelines = true;
        bool m_rebuildResources = true;

        // Configuration
        vk::Extent2D m_extent;
        vk::PolygonMode m_polygonMode = vk::PolygonMode::eFill;

        // Pass and subpasses
        vulkan::RenderPassHolder m_renderPassHolder;
        vulkan::PipelineHolder m_clearPipelineHolder;
        vulkan::PipelineHolder m_geometryPipelineHolder;
        vulkan::PipelineHolder m_depthlessPipelineHolder;
        vulkan::PipelineHolder m_epiphanyPipelineHolder;

        // GBuffer
        vulkan::DescriptorHolder m_gBufferInputDescriptorHolder;
        vk::DescriptorSet m_gBufferInputDescriptorSet = nullptr;
        std::vector<std::unique_ptr<vulkan::ImageHolder>> m_gBufferInputNodeImageHolders;

        vulkan::DescriptorHolder m_gBufferSsboDescriptorHolder;
        vk::DescriptorSet m_gBufferSsboDescriptorSet = nullptr;
        vulkan::BufferHolder m_gBufferSsboHeaderBufferHolder;
        vulkan::BufferHolder m_gBufferSsboListBufferHolder;

        // Resources
        vulkan::ImageHolder m_finalImageHolder;
        vulkan::ImageHolder m_depthImageHolder;
        vulkan::Framebuffer m_framebuffer;
    };
}
