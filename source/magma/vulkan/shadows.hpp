#pragma once

#include <lava/magma/render-scenes/render-scene.hpp>

#include "./holders/ubo-holder.hpp"
#include "./ubos.hpp"

namespace lava::magma {
    /**
     * Holds the data needed to bind shadows to a pipeline.
     * The ShadowsStage generates the images binded here.
     *
     * There should be one Shadows class per combinaison of light/camera.
     */
    class Shadows final {
    public:
        Shadows(RenderScene::Impl& scene);
        ~Shadows();

        void init(uint32_t lightId, uint32_t cameraId);
        void update(uint32_t frameId);
        void render(vk::CommandBuffer commandBuffer, uint32_t frameId, vk::PipelineLayout pipelineLayout,
                    uint32_t descriptorSetIndex) const;

        float cascadeSplitDepth(uint32_t cascadeIndex) const { return m_cascades[cascadeIndex].splitDepth; }
        const glm::mat4& cascadeTransform(uint32_t cascadeIndex) const { return m_cascades[cascadeIndex].transform; }

    protected:
        void updateImagesBindings();
        void updateBindings(uint32_t frameId);

    protected:
        struct Cascade {
            float splitDepth;
            glm::mat4 transform;
        };

    private:
        RenderScene::Impl& m_scene;
        uint32_t m_lightId = -1u;
        uint32_t m_cameraId = -1u;
        bool m_initialized = false;

        // Resources
        std::vector<vulkan::UboHolder> m_uboHolders;
        std::vector<vk::DescriptorSet> m_descriptorSets;
        std::array<Cascade, SHADOWS_CASCADES_COUNT> m_cascades;
    };
}
