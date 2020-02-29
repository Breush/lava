#include "./swapchain.hpp"

using namespace lava::magma;

vulkan::SwapchainSupportDetails vulkan::swapchainSupportDetails(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
    SwapchainSupportDetails details;
    details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface).value;
    details.formats = physicalDevice.getSurfaceFormatsKHR(surface).value;
    details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface).value;

    return details;
}
