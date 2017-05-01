#pragma once

#include "./tools.hpp"

namespace lava::vulkan {
    /**
     * Holds Vulkan queue family indices.
     */
    class QueueFamilyIndices {
    public:
        int graphics = -1;

        bool isComplete() { return graphics >= 0; }
    };

    /**
     * Find correct queue families of a device.
     */
    inline QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        auto queueFamilies = availableQueueFamilies(device);
        for (int i = 0; i < queueFamilies.size(); ++i) {
            const auto& queueFamily = queueFamilies[i];

            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphics = i;
            }

            if (indices.isComplete()) break;
        }

        return indices;
    }
}
