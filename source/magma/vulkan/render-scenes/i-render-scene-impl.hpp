#pragma once

#include <lava/magma/render-scenes/i-render-scene.hpp>

#include <vulkan/vulkan.hpp>

namespace lava::magma {
    class IRenderScene::Impl {
    public:
        virtual ~Impl() = default;

        // IRenderScene
        virtual Extent2d extent() const = 0;
        virtual void extent(Extent2d extent) = 0;

        /// Initialize the scene.
        virtual void init() = 0;

        /// Render the scene.
        virtual void render(vk::CommandBuffer commandBuffer) = 0;

        /// The final rendered image view.
        virtual vk::ImageView renderedImageView() const = 0;
    };
}
