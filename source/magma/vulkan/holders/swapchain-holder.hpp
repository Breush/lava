#pragma once

#include <lava/magma/render-engine.hpp>

#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    /**
     * An abstraction over a vk::SwapchainKHR.
     */
    class SwapchainHolder {
    public:
        SwapchainHolder(const RenderEngine::Impl& engine);

        /// Set up the swapchain given a valid surface.
        void init(vk::SurfaceKHR surface, const vk::Extent2D& windowExtent);

        /// Recreate the swapchain (the extent or the surface changed).
        void recreate(vk::SurfaceKHR surface, const vk::Extent2D& windowExtent);

        /// Acquire the next image.
        vk::Result acquireNextImage();

        //----- Getters

        const vk::SwapchainKHR& swapchain() const { return m_swapchain.get(); }

        /// Current index within the framebuffers.
        uint32_t currentIndex() const { return m_currentIndex; }

        /// The semaphore used to signal the next image is available.
        vk::Semaphore imageAvailableSemaphore() const { return m_imageAvailableSemaphore.get(); }

        /// The format chosen during initialization.
        vk::Format imageFormat() const { return m_imageFormat; }

        /// All the image views.
        const std::vector<vk::UniqueImageView>& imageViews() const { return m_imageViews; }

        /// Count of all the images. Same as `imageViews().size()`.
        uint32_t imagesCount() const { return m_images.size(); }

    protected:
        void createSwapchain(vk::SurfaceKHR surface, const vk::Extent2D& windowExtent);
        void createImageViews();
        void createSemaphore();

    private:
        // References
        const RenderEngine::Impl& m_engine;

        // Configuration
        $attribute(vk::Extent2D, extent);

        // Resources
        vk::UniqueSwapchainKHR m_swapchain;
        std::vector<vk::Image> m_images;
        std::vector<vk::UniqueImageView> m_imageViews;
        vk::UniqueSemaphore m_imageAvailableSemaphore;

        // Data
        uint32_t m_currentIndex = 0u;
        vk::Format m_imageFormat = vk::Format::eUndefined;
    };
}
