#pragma once

#include <lava/magma/render-image.hpp>

#include "../pipeline-kind.hpp"

namespace lava::magma {
    class Camera;
    class Scene;
}

namespace lava::magma {
    class CameraAft {
    public:
        CameraAft(Camera& fore, Scene& scene);

        void render(vk::CommandBuffer commandBuffer, PipelineKind pipelineKind, vk::PipelineLayout pipelineLayout, uint32_t pushConstantOffset) const;
        void changeImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer);

        // ----- Fore
        RenderImage foreRenderImage() const;
        RenderImage foreDepthRenderImage() const;
        void foreExtentChanged();
        void forePolygonModeChanged();

    private:
        Camera& m_fore;
        Scene& m_scene;
    };
}
