#pragma once

#include <lava/magma/render-engine.hpp>

#include "./wrappers.hpp"

namespace lava::magma::vulkan {
    /**
     * An abstraction over a vk::SwapchainKHR.
     */
    class SwapchainHolder {
    public:
        SwapchainHolder(const RenderEngine::Impl& engine);

        /// Set up the swapchain given a valid surface.
        void init(vk::SurfaceKHR surface, vk::Extent2D& windowExtent);

        /// Acquire the next image.
        vk::Result acquireNextImage();

        //----- Getters

        /// Current index within the framebuffers.
        uint32_t currentIndex() const { return m_currentIndex; }

        /// The semaphore used to signal the next image is available.
        vulkan::Semaphore& imageAvailableSemaphore() { return m_imageAvailableSemaphore; }

        /// The format chosen during initialization.
        vk::Format& imageFormat() { return m_imageFormat; }

        /// All the image views.
        std::vector<vulkan::ImageView>& imageViews() { return m_imageViews; }

        /// Count of all the images. Same as `imageViews().size()`.
        uint32_t imagesCount() const { return m_images.size(); }

        //----- Casts

        operator vk::SwapchainKHR&() { return m_swapchain; }
        operator const vk::SwapchainKHR&() const { return m_swapchain; }

    protected:
        void createSwapchain(vk::SurfaceKHR surface, vk::Extent2D& windowExtent);
        void createImageViews();
        void createSemaphore();

    private:
        // References
        const RenderEngine::Impl& m_engine;

        // Configuration
        $attribute(vk::Extent2D, extent);

        // Resources
        $attribute(vulkan::SwapchainKHR, swapchain);
        std::vector<vk::Image> m_images;
        std::vector<vulkan::ImageView> m_imageViews;
        vulkan::Semaphore m_imageAvailableSemaphore;

        // Data
        uint32_t m_currentIndex = 0u;
        vk::Format m_imageFormat;
    };
}
