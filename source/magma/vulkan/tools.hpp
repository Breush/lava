#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace lava::vulkan {
    /**
     * Listing some support.
     */
    inline std::vector<VkExtensionProperties> availableExtensions()
    {
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> properties(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());
        return properties;
    }

    /**
     * Listing some support.
     */
    inline std::vector<VkLayerProperties> availableLayers()
    {
        uint32_t count = 0;
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        std::vector<VkLayerProperties> properties(count);
        vkEnumerateInstanceLayerProperties(&count, properties.data());
        return properties;
    }
}
