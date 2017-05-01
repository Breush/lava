#pragma once

#include <vulkan/vulkan.hpp>

#include "./queue.hpp"

namespace lava::vulkan {
    /**
     * Checks if a device is suitable for our operations.
     */
    inline bool deviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        QueueFamilyIndices indices = findQueueFamilies(device, surface);
        return indices.isComplete();
    }
}
