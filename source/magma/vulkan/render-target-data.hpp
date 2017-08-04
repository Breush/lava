#pragma once

#include "./swapchain.hpp"

namespace lava::magma {
    // @fixme Rename accordingly
    struct DataRenderTarget {
        vulkan::Swapchain& swapchain; ///< Reference to the swapchain.
        vk::SurfaceKHR& surface;      ///< Reference to the surface.
    };

    struct InDataRenderTargetDraw {
        vk::Semaphore& renderFinishedSemaphore; ///< Reference to the semaphore called when the rendering is finished.
    };
}
