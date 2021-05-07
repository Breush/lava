#pragma once

#include "./i-renderer-stage.hpp"

#include <lava/magma/scene.hpp>

#include <lava/magma/ubos.hpp>

#include "../holders/image-holder.hpp"
#include "../holders/pipeline-holder.hpp"
#include "../holders/render-pass-holder.hpp"
#include "../holders/descriptor-holder.hpp"
#include "../holders/acceleration-structure-holder.hpp"

namespace lava::magma {
    /**
     * Ray tracer stage.
     */
    class RayTracerStage final : public IRendererStage {
        constexpr static const uint32_t CAMERA_PUSH_CONSTANT_OFFSET = 0u;

    public:
        RayTracerStage(Scene& scene);

        // IRendererStage
        void init(const Camera& camera) final;
        void rebuild() final;
        void record(vk::CommandBuffer commandBuffer, uint32_t frameId) final;

        void extent(const vk::Extent2D& extent) final;
        void sampleCount(vk::SampleCountFlagBits /* sampleCount */) final {}
        void polygonMode(vk::PolygonMode /* polygonMode */) final {}

        RenderImage renderImage() const final;
        RenderImage depthRenderImage() const final { return RenderImage(); }
        bool depthRenderImageValid() const final { return false; }
        vk::RenderPass renderPass() const final { return nullptr; }

        void changeRenderImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer) final;

    protected:
        void initPass();

        void updatePassShaders(bool /* firstTime */) {} // @fixme

        void createAS();
        void createResources();

    private:
        // References
        Scene& m_scene;
        const Camera* m_camera = nullptr;

        bool m_rebuildPipelines = true;
        bool m_rebuildResources = true;

        // Configuration
        vk::Extent2D m_extent;

        // Pass and subpasses
        vulkan::PipelineHolder m_pipelineHolder;

        // Resources
        vulkan::ImageHolder m_finalImageHolder;

        // @fixme Should be stored in Scene! (Or in AS Holder?)
        vulkan::AccelerationStructureHolder topLevelAS;
        vulkan::BufferHolder indicesBuffer;
        vulkan::BufferHolder verticesBuffer;
        vulkan::DescriptorHolder descriptorHolder;
        vk::UniqueDescriptorSet descriptorSet;

        // @fixme This is for us, though, add m_
        vulkan::BufferHolder raygenShaderBindingTable;
        vulkan::BufferHolder missShaderBindingTable;
        vulkan::BufferHolder hitShaderBindingTable;
        vk::StridedDeviceAddressRegionKHR raygenShaderSbtEntry;
        vk::StridedDeviceAddressRegionKHR missShaderSbtEntry;
        vk::StridedDeviceAddressRegionKHR hitShaderSbtEntry;
        vk::StridedDeviceAddressRegionKHR callableShaderSbtEntry;
    };
}
