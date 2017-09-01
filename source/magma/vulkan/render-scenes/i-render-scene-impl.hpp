#pragma once

#include <lava/magma/render-scenes/i-render-scene.hpp>

#include <vulkan/vulkan.hpp>

namespace lava::magma {
    class IRenderScene::Impl {
    public:
        virtual ~Impl() = default;

        /// Initialize the scene.
        virtual void init(uint32_t id) = 0;

        /// Render the scene.
        virtual void render(vk::CommandBuffer commandBuffer) = 0;

        /// The final rendered image view.
        virtual vk::ImageView renderedImageView(uint32_t cameraIndex) const = 0;
    };
}
