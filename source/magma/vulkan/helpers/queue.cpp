#include "./queue.hpp"

using namespace lava::magma;

vulkan::QueueFamilyIndices vulkan::findQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR* pSurface)
{
    QueueFamilyIndices indices;

    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    for (size_t i = 0; i < queueFamilies.size(); ++i) {
        const auto& queueFamily = queueFamilies[i];
        if (queueFamily.queueCount <= 0) continue;

        vk::Bool32 presentSupport = false;

        // Graphics
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphics = i;

            // If we don't need present support, the graphics one will do.
            if (pSurface == nullptr) {
                presentSupport = true;
            }
        }

        // Check that it can handle surfaces
        if (pSurface != nullptr) {
            physicalDevice.getSurfaceSupportKHR(i, *pSurface, &presentSupport);
        }

        if (presentSupport) {
            indices.present = i;
        }

        // For transfer, take one that is not graphics or present if possible.
        if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer && indices.graphics != int(i) && indices.present != int(i)) {
            indices.transfer = i;
        }
    }

    // Fallback to the present indices if no better.
    if (indices.transfer < 0) {
        indices.transfer = indices.present;
    }

    return indices;
}
