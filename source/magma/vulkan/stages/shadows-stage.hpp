#pragma once

#include <lava/magma/render-scenes/render-scene.hpp>

#include <lava/magma/ubos.hpp>

#include "../holders/image-holder.hpp"
#include "../holders/pipeline-holder.hpp"
#include "../holders/render-pass-holder.hpp"
#include "../render-engine-impl.hpp"

namespace lava::magma {
    // @todo Currently fixed extent for shadow maps, might need dynamic ones
    constexpr const uint32_t SHADOW_MAP_SIZE = 2048u;

    /**
     * Creating a shadows maps for the lights.
     */
    class ShadowsStage final {
        constexpr static const uint32_t MESH_PUSH_CONSTANT_OFFSET = 0u;
        constexpr static const uint32_t SHADOW_MAP_PUSH_CONSTANT_OFFSET = sizeof(MeshUbo);

    public:
        ShadowsStage(RenderScene::Impl& scene);

        void init(uint32_t lightId);
        void update(vk::Extent2D extent);
        void updateFromCamerasCount();
        void render(vk::CommandBuffer commandBuffer, uint32_t cameraId);

        RenderImage renderImage(uint32_t cameraId = 0u, uint32_t cascadeIndex = 0u) const;
        vk::RenderPass renderPass() const { return m_renderPassHolder.renderPass(); }

    protected:
        void initPass();

        void createResources();
        void ensureResourcesForCamera(uint32_t cameraId);

    protected:
        struct Cascade {
            vulkan::ImageHolder imageHolder;
            vulkan::Framebuffer framebuffer;
            ShadowMapUbo ubo;

            Cascade(RenderEngine::Impl& engine)
                : imageHolder(engine, "magma.vulkan.shadows-stage.cascade.image")
                , framebuffer(engine.device())
            {
            }
        };

    private:
        // References
        RenderScene::Impl& m_scene;
        uint32_t m_lightId = -1u;
        vk::Extent2D m_extent;

        // Pass and subpasses
        vulkan::RenderPassHolder m_renderPassHolder;
        vulkan::PipelineHolder m_pipelineHolder;

        // Cascade shadow maps
        // @todo For clarity, we might want a using CameraId = uint32_t somewhere.
        std::unordered_map<uint32_t, std::vector<Cascade>> m_cascades; // Stored per cameraId
    };
}
