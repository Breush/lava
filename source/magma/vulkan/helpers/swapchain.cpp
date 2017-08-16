#include "./swapchain.hpp"

using namespace lava::magma;

vulkan::SwapchainSupportDetails vulkan::swapchainSupportDetails(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
    SwapchainSupportDetails details;
    details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    details.formats = physicalDevice.getSurfaceFormatsKHR(surface);
    details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

    return details;
}
