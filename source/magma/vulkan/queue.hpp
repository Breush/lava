#pragma once

#include "./tools.hpp"

namespace lava::vulkan {
    /**
     * Holds Vulkan queue family indices.
     */
    class QueueFamilyIndices {
    public:
        int graphics = -1;
        int present = -1;

        bool isComplete() { return graphics >= 0 && present >= 0; }
    };

    /**
     * Find correct queue families of a device.
     */
    inline QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        QueueFamilyIndices indices;

        auto queueFamilies = availableQueueFamilies(device);
        for (int i = 0; i < queueFamilies.size(); ++i) {
            const auto& queueFamily = queueFamilies[i];
            if (queueFamily.queueCount <= 0) continue;

            // Graphics
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphics = i;
            }

            // Check that it can handle surfaces
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                indices.present = i;
            }

            if (indices.isComplete()) break;
        }

        return indices;
    }
}
