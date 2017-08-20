#pragma once

#include <lava/magma/render-targets/i-render-target.hpp>

#include <cstdint>
#include <vulkan/vulkan.hpp>

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

        /// Prepare the upcoming draw.
        virtual void prepare() = 0;

        /// Draw.
        virtual void draw(vk::Semaphore renderFinishedSemaphore) const = 0;

        /// The swapchain holder used.
        virtual const vulkan::SwapchainHolder& swapchainHolder() const = 0;

        /// The surface used.
        virtual vk::SurfaceKHR surface() const = 0;
    };
}
