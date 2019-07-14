#pragma once

#include <lava/magma/render-scenes/render-scene.hpp>

#include "../vulkan/holders/image-holder.hpp"

namespace lava::magma {
    class Camera;
}

namespace lava::magma {
    class CameraAft {
    public:
        CameraAft(Camera& fore, RenderScene::Impl& scene);

        void init(uint32_t id);
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset) const;
        void changeImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer);

        // ----- Fore
        RenderImage foreRenderImage() const;
        RenderImage foreDepthRenderImage() const;
        void foreExtentChanged();
        void forePolygonModeChanged();

    private:
        Camera& m_fore;
        RenderScene::Impl& m_scene;

        // ----- Infos
        uint32_t m_id = -1u;
    };
}
