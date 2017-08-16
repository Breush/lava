#pragma once

namespace lava::magma::vulkan {
    class SwapchainHolder;
}

namespace lava::magma {
    struct DataRenderTarget {
        vulkan::SwapchainHolder& swapchainHolder; ///< Reference to the swapchain holder.
        vk::SurfaceKHR& surface;                  ///< Reference to the surface.
    };

    struct InDataRenderTargetInit {
        uint32_t id; ///< Id to communicate with Engine::Impl.
    };

    struct InDataRenderTargetDraw {
        vk::Semaphore& renderFinishedSemaphore; ///< Reference to the semaphore called when the rendering is finished.
    };
}
