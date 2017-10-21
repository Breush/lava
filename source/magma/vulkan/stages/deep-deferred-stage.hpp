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
        constexpr static const uint32_t GBUFFER_DESCRIPTOR_SET_INDEX = 0u;
        constexpr static const uint32_t CAMERA_DESCRIPTOR_SET_INDEX = 1u;
        constexpr static const uint32_t MATERIAL_DESCRIPTOR_SET_INDEX = 2u;
        constexpr static const uint32_t MESH_DESCRIPTOR_SET_INDEX = 3u;

        constexpr static const uint32_t EPIPHANY_DESCRIPTOR_SET_INDEX = 2u;

        struct GBufferNode {
            // 26 bits can handle 8K resolution
            // 6 bits allows 64 different material shaders
            uint32_t materialId6_next26;
            float depth;
            uint32_t materialData[9];
        };

    public:
        DeepDeferredStage(RenderScene::Impl& scene);

        void init(uint32_t cameraId);
        void update(vk::Extent2D extent);
        void render(vk::CommandBuffer commandBuffer);

        const vulkan::ImageView& imageView() const { return m_finalImageHolder.view(); }

    protected:
        void initGBuffer();
        void initClearPass();
        void initGeometryOpaquePass();
        void initGeometryTranslucentPass();
        void initEpiphanyPass();

        void createResources();
        void createFramebuffers();

        void updateEpiphanyUbo();

    private:
        // References
        RenderScene::Impl& m_scene;
        uint32_t m_cameraId = -1u;
        vk::Extent2D m_extent;

        // Pass and subpasses
        vulkan::RenderPassHolder m_renderPassHolder;
        vulkan::PipelineHolder m_clearPipelineHolder;
        vulkan::PipelineHolder m_geometryOpaquePipelineHolder;
        vulkan::PipelineHolder m_geometryTranslucentPipelineHolder;
        vulkan::PipelineHolder m_epiphanyPipelineHolder;

        // Epiphany
        vulkan::DescriptorHolder m_epiphanyDescriptorHolder;
        vk::DescriptorSet m_epiphanyDescriptorSet = nullptr;
        vulkan::UboHolder m_epiphanyUboHolder;

        // GBuffer
        vulkan::DescriptorHolder m_gBufferDescriptorHolder;
        vk::DescriptorSet m_gBufferDescriptorSet = nullptr;
        vulkan::BufferHolder m_gBufferHeaderBufferHolder;
        vulkan::BufferHolder m_gBufferListBufferHolder;

        // Resources
        vulkan::ImageHolder m_finalImageHolder;
        vulkan::ImageHolder m_depthImageHolder;
        vulkan::Framebuffer m_framebuffer;
    };
}
