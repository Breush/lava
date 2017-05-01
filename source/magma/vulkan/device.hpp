#pragma once

#include <vulkan/vulkan.hpp>

#include "./queue.hpp"

namespace lava::vulkan {
    /**
     * Checks if a device is suitable for our operations.
     */
    inline bool deviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = findQueueFamilies(device);
        return indices.isComplete();
    }
}
