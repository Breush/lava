#pragma once

#include <set>
#include <vulkan/vulkan.hpp>

#include "./queue.hpp"
#include "./swap-chain.hpp"
#include "./tools.hpp"

namespace lava::vulkan {
    /**
     * Checks if a device supports the extensions.
     */
    inline bool deviceExtensionsSupported(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions)
    {
        auto extensions = availableExtensions(device);
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : extensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    /**
     * Checks if a device is suitable for our operations.
     */
    inline bool deviceSuitable(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions, VkSurfaceKHR surface)
    {
        QueueFamilyIndices indices = findQueueFamilies(device, surface);
        if (!indices.valid()) return false;

        auto extensionsSupported = deviceExtensionsSupported(device, deviceExtensions);
        if (!extensionsSupported) return false;

        auto swapChainSupport = swapChainSupported(device, surface);
        if (!swapChainSupport.valid()) false;

        return true;
    }
}
