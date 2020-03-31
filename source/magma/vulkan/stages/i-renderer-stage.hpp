#pragma once

#include <lava/magma/render-image.hpp>

namespace lava::magma {
    class Camera;
}

namespace lava::magma {
    /// Interface for renderer stages, as used by RenderScene::Impl.
    class IRendererStage {
    public:
        virtual ~IRendererStage() = default;

        virtual void init(const Camera& camera) = 0;
        virtual void rebuild() = 0;
        virtual void render(vk::CommandBuffer commandBuffer, uint32_t frameId) = 0;

        virtual void extent(vk::Extent2D extent) = 0;
        virtual void sampleCount(vk::SampleCountFlagBits sampleCount) = 0;
        virtual void polygonMode(vk::PolygonMode polygonMode) = 0;

        virtual RenderImage renderImage() const = 0;
        virtual RenderImage depthRenderImage() const = 0;
        virtual vk::RenderPass renderPass() const = 0;

        virtual void changeRenderImageLayout(vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer) = 0;
    };
}
