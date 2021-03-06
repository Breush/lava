#pragma once

#include <lava/magma/render-image.hpp>

#include "../vulkan/holders/ubo-holder.hpp"
#include "./config.hpp"

namespace lava::magma {
    class Light;
    class Scene;
}

namespace lava::magma {
    class LightAft {
    public:
        LightAft(Light& fore, Scene& scene);

        void init();
        void update();
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex) const;

        // ----- Fore
        RenderImage foreShadowsRenderImage() const;
        void foreUboChanged() { m_uboDirty = true; }

    protected:
        void updateBindings();

    private:
        Light& m_fore;
        Scene& m_scene;

        uint32_t m_currentFrameId = 0u;

        // ----- Shader data
        std::array<vk::UniqueDescriptorSet, FRAME_IDS_COUNT> m_descriptorSets;
        std::array<vulkan::UboHolder, FRAME_IDS_COUNT> m_uboHolders;
        bool m_uboDirty = false;
    };
}
