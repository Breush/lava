#pragma once

#include <lava/magma/render-image.hpp>

namespace lava::magma {
    /// Interface for renderer stages, as used by RenderScene::Impl.
    class IRendererStage {
    public:
        virtual ~IRendererStage() = default;

        virtual void init(uint32_t cameraId) = 0;
        virtual void update(vk::Extent2D extent, vk::PolygonMode polygonMode = vk::PolygonMode::eFill) = 0;
        virtual void render(vk::CommandBuffer commandBuffer) = 0;

        virtual RenderImage renderImage() const = 0;
        virtual RenderImage depthRenderImage() const = 0;

        virtual void changeRenderImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer) = 0;
    };
}
