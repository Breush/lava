#pragma once

namespace lava::magma::vulkan {
    /**
     * Holds Vulkan queue family indices.
     */
    class QueueFamilyIndices {
    public:
        int graphics = -1;
        int present = -1;

        bool valid() { return graphics >= 0 && present >= 0; }
    };

    /**
     * Find correct queue families of a device.
     */
    inline QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
    {
        QueueFamilyIndices indices;

        auto queueFamilies = physicalDevice.getQueueFamilyProperties();
        for (size_t i = 0; i < queueFamilies.size(); ++i) {
            const auto& queueFamily = queueFamilies[i];
            if (queueFamily.queueCount <= 0) continue;

            // Graphics
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphics = i;
            }

            // Check that it can handle surfaces
            vk::Bool32 presentSupport = false;
            physicalDevice.getSurfaceSupportKHR(i, surface, &presentSupport);
            if (presentSupport) {
                indices.present = i;
            }

            if (indices.valid()) break;
        }

        return indices;
    }
}
