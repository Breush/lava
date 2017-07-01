#pragma once

#include <vulkan/vulkan.hpp>

#include "./capsule.hpp"

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

        // ----- Getters

        Capsule<VkSwapchainKHR>& capsule() { return m_swapchain; }
        VkFormat& imageFormat() { return m_imageFormat; }
        VkExtent2D& extent() { return m_extent; }
        std::vector<Capsule<VkImageView>>& imageViews() { return m_imageViews; }

        operator VkSwapchainKHR() const { return m_swapchain; }

    protected:
        void createSwapchain(VkSurfaceKHR surface, VkExtent2D& windowExtent);
        void createImageViews();

    private:
        Capsule<VkSwapchainKHR> m_swapchain;
        std::vector<VkImage> m_images;
        VkFormat m_imageFormat;
        VkExtent2D m_extent;
        std::vector<Capsule<VkImageView>> m_imageViews;

        Device& m_device;
    };
}
