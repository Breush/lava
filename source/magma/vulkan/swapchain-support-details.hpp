#pragma once

#include <vulkan/vulkan.hpp>

namespace lava::magma::vulkan {
    class SwapchainSupportDetails {
    public:
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;

        bool valid() { return !formats.empty() && !presentModes.empty(); }
    };

    inline SwapchainSupportDetails swapchainSupportDetails(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
    {
        SwapchainSupportDetails details;
        details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
        details.formats = physicalDevice.getSurfaceFormatsKHR(surface);
        details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

        return details;
    }
}
