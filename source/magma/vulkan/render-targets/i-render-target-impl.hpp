#pragma once

#include <lava/magma/render-targets/i-render-target.hpp>

namespace lava::magma::vulkan {
    class SwapchainHolder;
}

namespace lava::magma {
    /**
     * Interface for render targets.
     */
    class IRenderTarget::Impl {
    public:
        virtual ~Impl() = default;

        /// Called once when added to the engine.
        virtual void init(uint32_t id) = 0;

        /// Update any internal state.
        virtual void update() = 0;

        /// Prepare the upcoming draw. Returns whether the engine can draw next.
        virtual bool prepare() = 0;

        /// Render. Called to fill the commandBuffer with last specific commands of the render target.
        virtual void render(vk::CommandBuffer commandBuffer) = 0;

        /// Draw. Called to show/present the final image. Should submit the commandBuffers.
        virtual void draw(const std::vector<vk::CommandBuffer>& commandBuffers) const = 0;

        /// The id used as initialization.
        virtual uint32_t id() const = 0;

        /// The index of the current buffer, as there will be as many commandBuffers as needed.
        virtual uint32_t currentBufferIndex() const = 0;

        /// The index of the current buffer, as there will be as many commandBuffers as needed.
        virtual uint32_t buffersCount() const = 0;

        /// Render an image in a specific viewport.
        virtual uint32_t addView(vk::ImageView imageView, vk::ImageLayout imageLayout, vk::Sampler sampler,
                                 Viewport viewport, uint32_t channelCount) = 0;

        /// Remove the specified view.
        virtual void removeView(uint32_t viewId) = 0;

        /// Update the image of the specified view.
        virtual void updateView(uint32_t viewId, vk::ImageView imageView, vk::ImageLayout imageLayout, vk::Sampler sampler) = 0;
    };
}
