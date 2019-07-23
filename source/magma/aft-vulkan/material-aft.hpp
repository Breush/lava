#pragma once

#include "../vulkan/holders/ubo-holder.hpp"
#include "./config.hpp"

namespace lava::magma {
    class Material;
    class Scene;
}

namespace lava::magma {
    class MaterialAft {
    public:
        MaterialAft(Material& fore, Scene& scene);
        ~MaterialAft();

        void init();
        void update();
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t descriptorSetIndex) const;

        // ----- Fore
        void foreUboChanged() { m_uboDirty = true; }

    protected:
        void updateBindings();

    private:
        Material& m_fore;
        Scene& m_scene;

        uint32_t m_currentFrameId = 0u;

        // ----- Shader data
        std::array<vk::DescriptorSet, FRAME_IDS_COUNT> m_descriptorSets;
        std::array<vulkan::UboHolder, FRAME_IDS_COUNT> m_uboHolders;
        bool m_uboDirty = false;
    };
}
