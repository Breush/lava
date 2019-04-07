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

            // We don't need present support, the graphics one will do.
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
    }

    return indices;
}
