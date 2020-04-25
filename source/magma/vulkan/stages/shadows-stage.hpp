#pragma once

#include <lava/magma/scene.hpp>

#include <lava/magma/ubos.hpp>

#include "../holders/image-holder.hpp"
#include "../holders/pipeline-holder.hpp"
#include "../holders/render-pass-holder.hpp"

namespace lava::magma {
    /**
     * Creating a shadows maps for the lights.
     */
    class ShadowsStage final {
        constexpr static const uint32_t MESH_PUSH_CONSTANT_OFFSET = 0u;
        constexpr static const uint32_t SHADOW_MAP_PUSH_CONSTANT_OFFSET = sizeof(MeshUbo);

    public:
        ShadowsStage(Scene& scene);

        void init(const Light& light);
        void update(vk::Extent2D extent);
        void updateFromCamerasCount();
        void record(vk::CommandBuffer commandBuffer, const Camera* camera);

        RenderImage renderImage(const Camera& camera, uint32_t cascadeIndex = 0u) const;
        vk::RenderPass renderPass() const { return m_renderPassHolder.renderPass(); }

    protected:
        void initPass();

        void createResources();
        void ensureResourcesForCamera(const Camera& camera);

    protected:
        struct Cascade {
            // @fixme Why not unique_ptr, exactly?
            std::shared_ptr<vulkan::ImageHolder> imageHolder;
            std::shared_ptr<vulkan::Framebuffer> framebuffer;
            ShadowMapUbo ubo;

            Cascade(RenderEngine& engine);
        };

    private:
        // References
        Scene& m_scene;
        const Light* m_light = nullptr;
        vk::Extent2D m_extent = {0, 0};

        // Pass and subpasses
        vulkan::RenderPassHolder m_renderPassHolder;
        vulkan::PipelineHolder m_pipelineHolder;

        // Cascade shadow maps
        // @todo For clarity, we might want a using CameraId = uint32_t somewhere.
        std::unordered_map<const Camera*, std::vector<Cascade>> m_cascades; // Stored per cameraId
    };
}
