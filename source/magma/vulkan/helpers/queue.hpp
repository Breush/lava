#pragma once

namespace lava::magma::vulkan {
    /// Holds queue family indices.
    class QueueFamilyIndices {
    public:
        int graphics = -1;
        int transfer = -1;
        int present = -1;

        bool valid() { return graphics >= 0 && transfer >= 0 && present >= 0; }
    };

    /// Find correct queue families of a device.
    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR* pSurface = nullptr);
}
