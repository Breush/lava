#pragma once

#include <glm/vec3.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>

#include "../holders/buffer-holder.hpp"
#include "../holders/descriptor-holder.hpp"
#include "../holders/image-holder.hpp"
#include "../holders/pipeline-holder.hpp"
#include "../holders/render-pass-holder.hpp"
#include "../holders/ubo-holder.hpp"

namespace lava::magma {
    /**
     * Pipeline layout for the deep deferred rendering.
     * It constructs a linked-list deep GBuffer.
     */
    class DeepDeferredStage final {
        constexpr static const uint32_t DEEP_DEFERRED_GBUFFER_MAX_NODE_DEPTH = 3u;
        constexpr static const uint32_t DEEP_DEFERRED_GBUFFER_NODE_MATERIAL_DATA_SIZE = 12u;
        constexpr static const uint32_t DEEP_DEFERRED_GBUFFER_RENDER_TARGETS_COUNT = 4u;

        constexpr static const uint32_t DEEP_DEFERRED_GBUFFER_INPUT_DESCRIPTOR_SET_INDEX = 0u;
        constexpr static const uint32_t DEEP_DEFERRED_GBUFFER_SSBO_DESCRIPTOR_SET_INDEX = 1u;

        constexpr static const uint32_t CAMERA_DESCRIPTOR_SET_INDEX = 2u;

        constexpr static const uint32_t GEOMETRY_MATERIAL_DESCRIPTOR_SET_INDEX = 3u;
        constexpr static const uint32_t GEOMETRY_MESH_DESCRIPTOR_SET_INDEX = 4u;

        constexpr static const uint32_t EPIPHANY_LIGHTS_DESCRIPTOR_SET_INDEX = 3u;

        struct GBufferNode {
            // 26 bits can handle 8K resolution
            // 6 bits allows 64 different material shaders
            uint32_t materialId6_next26;
            float depth;
            uint8_t materialData[DEEP_DEFERRED_GBUFFER_NODE_MATERIAL_DATA_SIZE];
        };

    public:
        DeepDeferredStage(RenderScene::Impl& scene);

        void init(uint32_t cameraId);
        void update(vk::Extent2D extent);
        void render(vk::CommandBuffer commandBuffer);

        RenderImage renderImage() const;
        RenderImage depthRenderImage() const;

    protected:
        void initGBuffer();
        void initClearPass();
        void initGeometryPass();
        void initGeometryTranslucentPass();
        void initEpiphanyPass();

        void updateGeometryPassShaders(bool firstTime);
        void updateEpiphanyPassShaders(bool firstTime);

        void createResources();
        void createFramebuffers();

        void updateEpiphanyLightsBindings();

    private:
        // References
        RenderScene::Impl& m_scene;
        uint32_t m_cameraId = -1u;
        vk::Extent2D m_extent;

        // Pass and subpasses
        vulkan::RenderPassHolder m_renderPassHolder;
        vulkan::PipelineHolder m_clearPipelineHolder;
        vulkan::PipelineHolder m_geometryPipelineHolder;
        vulkan::PipelineHolder m_epiphanyPipelineHolder;

        // Lights
        vulkan::DescriptorHolder m_lightsDescriptorHolder;
        vk::DescriptorSet m_lightsDescriptorSet = nullptr;
        vulkan::UboHolder m_lightsUboHolder;

        // GBuffer
        vulkan::DescriptorHolder m_gBufferInputDescriptorHolder;
        vk::DescriptorSet m_gBufferInputDescriptorSet = nullptr;
        std::vector<vulkan::ImageHolder> m_gBufferInputNodeImageHolders;

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
