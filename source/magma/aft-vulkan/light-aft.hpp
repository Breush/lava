#pragma once

#include <lava/magma/render-image.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>

#include "../vulkan/holders/ubo-holder.hpp"

namespace lava::magma {
    class Light;
}

namespace lava::magma {
    class LightAft {
    public:
        LightAft(Light& fore, RenderScene::Impl& scene);

        void init(uint32_t id);
        void update();
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex) const;

        // ----- Fore
        RenderImage foreShadowsRenderImage() const;
        void foreUboChanged() { m_uboDirty = true; }

    protected:
        void updateBindings();

    private:
        Light& m_fore;
        RenderScene::Impl& m_scene;

        uint32_t m_currentFrameId = 0u;

        // ----- Infos
        uint32_t m_id = -1u;

        // ----- Shader data
        std::array<vk::DescriptorSet, RenderScene::FRAME_IDS_COUNT> m_descriptorSets;
        std::array<vulkan::UboHolder, RenderScene::FRAME_IDS_COUNT> m_uboHolders;
        bool m_uboDirty = false;
    };
}
