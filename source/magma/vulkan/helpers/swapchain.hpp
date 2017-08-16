#pragma once

#include <vulkan/vulkan.hpp>

namespace lava::magma::vulkan {
    /// All details about the swapchain.
    class SwapchainSupportDetails {
    public:
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;

        bool valid() { return !formats.empty() && !presentModes.empty(); }
    };

    /// Get the swapchain details.
    SwapchainSupportDetails swapchainSupportDetails(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
}
