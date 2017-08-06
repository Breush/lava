#pragma once

#include "./swapchain-holder.hpp"

namespace lava::magma {
    struct DataRenderTarget {
        vulkan::SwapchainHolder& swapchainHolder; ///< Reference to the swapchain holder.
        vk::SurfaceKHR& surface;                  ///< Reference to the surface.
    };

    struct InDataRenderTargetDraw {
        vk::Semaphore& renderFinishedSemaphore; ///< Reference to the semaphore called when the rendering is finished.
    };
}
