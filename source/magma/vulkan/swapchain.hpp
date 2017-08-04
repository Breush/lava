#pragma once

#include <vulkan/vulkan.hpp>

#include "./capsule.hpp"
#include "./wrappers.hpp"

namespace lava::magma::vulkan {
    class Device;
}

namespace lava::magma::vulkan {
    /**
     * An abstraction over a VkSwapchainKHR.
     */
    class Swapchain {
    public:
        Swapchain(Device& device);

        void init(VkSurfaceKHR surface, VkExtent2D& windowExtent);
        vk::Result acquireNextImage();

        // ----- Getters

        /// Current index within the framebuffers.
        uint32_t currentIndex() const { return m_currentIndex; }

        /// The semaphore used to signal the next image is available.
        vulkan::Semaphore& imageAvailableSemaphore() { return m_imageAvailableSemaphore; }

        VkFormat& imageFormat() { return m_imageFormat; }
        VkExtent2D& extent() { return m_extent; }
        std::vector<Capsule<VkImageView>>& imageViews() { return m_imageViews; }
        uint32_t imagesCount() const { return m_imageViews.size(); }

    protected:
        void createSwapchain(VkSurfaceKHR surface, VkExtent2D& windowExtent);
        void createImageViews();
        void createSemaphore();

    private:
        Capsule<VkSwapchainKHR> m_swapchain;
        std::vector<VkImage> m_images;
        VkFormat m_imageFormat;
        VkExtent2D m_extent;
        std::vector<Capsule<VkImageView>> m_imageViews;
        uint32_t m_currentIndex = 0u;

        // References
        Device& m_device;

        // Resources
        vulkan::Semaphore m_imageAvailableSemaphore;
    };
}
