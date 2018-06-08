#pragma once

#include <lava/magma/render-scenes/i-render-scene.hpp>

#include <lava/magma/render-image.hpp>

#include <vulkan/vulkan.hpp>

namespace lava::magma {
    class IRenderScene::Impl {
    public:
        virtual ~Impl() = default;

        /// Initialize the scene.
        virtual void init(uint32_t id) = 0;

        /// Render the scene.
        virtual void render(vk::CommandBuffer commandBuffer) = 0;

        /// The final rendered image view for the specified camera.
        virtual RenderImage cameraRenderImage(uint32_t cameraIndex) const = 0;

        /// The shadow-map rendered image view for the specified camera.
        virtual RenderImage cameradepthRenderImage(uint32_t cameraIndex) const = 0;
    };
}
