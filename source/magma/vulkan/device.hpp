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

namespace lava {
    struct QueueFamilyIndices {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    };

    /**
     * Encapsulation of a Vulkan device.
     */
    class Device {
    public:
        Device();
        ~Device();

        inline VkDevice& logicalDevice() { return m_logicalDevice; }
        inline VkPhysicalDevice& physicalDevice() { return m_physicalDevice; }
        inline QueueFamilyIndices& queueFamilyIndices() { return m_queueFamilyIndices; }

        void bind(VkPhysicalDevice physicalDevice);

        bool extensionSupported(const std::string& extension);

        /**
        * Create the logical device based on the assigned physical device, also gets default queue family indices.
        */
        VkResult createLogicalDevice(VkPhysicalDeviceFeatures& enabledFeatures, std::vector<const char*>& enabledExtensions);

    protected:
        uint32_t getQueueFamilyIndex(VkQueueFlagBits queueFlags);

    private:
        VkDevice m_logicalDevice;
        VkPhysicalDevice m_physicalDevice;
        VkPhysicalDeviceProperties m_properties;
        VkPhysicalDeviceFeatures m_features;
        VkPhysicalDeviceMemoryProperties m_memoryProperties;
        std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
        std::vector<std::string> m_supportedExtensions;
        QueueFamilyIndices m_queueFamilyIndices;
    };
};
