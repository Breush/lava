#pragma once

#include <lava/magma/render-scenes/render-scene.hpp>
#include <vulkan/vulkan.hpp>

#include "../holders/image-holder.hpp"
#include "../holders/pipeline-holder.hpp"
#include "../holders/render-pass-holder.hpp"

namespace lava::magma {
    /**
     * Creating a shadows maps for the lights.
     */
    class ShadowsStage final {
        constexpr static const uint32_t LIGHTS_DESCRIPTOR_SET_INDEX = 0u;
        constexpr static const uint32_t MESH_DESCRIPTOR_SET_INDEX = 1u;

    public:
        ShadowsStage(RenderScene::Impl& scene);

        void init(uint32_t lightId);
        void update(vk::Extent2D extent);
        void render(vk::CommandBuffer commandBuffer);

        RenderImage renderImage() const;

    protected:
        void initPass();

        void createResources();
        void createFramebuffers();

    private:
        // References
        RenderScene::Impl& m_scene;
        uint32_t m_lightId = -1u;
        vk::Extent2D m_extent;

        // Pass and subpasses
        vulkan::RenderPassHolder m_renderPassHolder;
        vulkan::PipelineHolder m_pipelineHolder;

        // Resources
        vulkan::ImageHolder m_depthImageHolder;
        vulkan::Framebuffer m_framebuffer;
    };
}
