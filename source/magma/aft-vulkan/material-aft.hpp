#pragma once

#include <lava/magma/render-scenes/render-scene.hpp>
#include <lava/magma/ubos.hpp>

#include "../vulkan/holders/ubo-holder.hpp"

namespace lava::magma {
    class Material;
}

namespace lava::magma {
    class MaterialAft {
    public:
        MaterialAft(Material& fore, RenderScene::Impl& scene);
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
        RenderScene::Impl& m_scene;

        uint32_t m_currentFrameId = 0u;

        // ----- Shader data
        std::array<vk::DescriptorSet, RenderScene::FRAME_IDS_COUNT> m_descriptorSets;
        std::array<vulkan::UboHolder, RenderScene::FRAME_IDS_COUNT> m_uboHolders;
        bool m_uboDirty = false;
    };
}
