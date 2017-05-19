#include "./swapchain.hpp"

#include <lava/chamber/logger.hpp>

#include "./device.hpp"
#include "./image.hpp"
#include "./queue.hpp"
#include "./swapchain-support-details.hpp"

namespace {
    VkSurfaceFormatKHR swapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
            return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        }

        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
                && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR swapChainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
    {
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        for (const auto& availablePresentMode : availablePresentModes) {
            // We prefer triple-buffering over other present modes
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
            else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                bestMode = availablePresentMode;
            }
        }

        return bestMode;
    }

    VkExtent2D swapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowExtent)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        VkExtent2D actualExtent = windowExtent;
        actualExtent.width = std::max(capabilities.minImageExtent.width, actualExtent.width);
        actualExtent.height = std::max(capabilities.minImageExtent.height, actualExtent.height);
        actualExtent.width = std::min(capabilities.maxImageExtent.width, actualExtent.width);
        actualExtent.height = std::min(capabilities.maxImageExtent.height, actualExtent.height);
        return actualExtent;
    }
}

using namespace lava::vulkan;

Swapchain::Swapchain(Device& device)
    : m_swapchain{device.capsule(), vkDestroySwapchainKHR}
    , m_device(device)
{
}

void Swapchain::init(VkSurfaceKHR surface, VkExtent2D& windowExtent)
{
    createSwapchain(surface, windowExtent);
    createImageViews();
}

void Swapchain::createSwapchain(VkSurfaceKHR surface, VkExtent2D& windowExtent)
{
    auto details = swapchainSupportDetails(m_device.physicalDevice(), surface);

    auto surfaceFormat = swapChainSurfaceFormat(details.formats);
    auto presentMode = swapChainPresentMode(details.presentModes);
    auto extent = swapChainExtent(details.capabilities, windowExtent);

    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VkSwapchainKHR(m_swapchain);

    auto indices = findQueueFamilies(m_device.physicalDevice(), surface);
    std::vector<uint32_t> queueFamilyIndices = {(uint32_t)indices.graphics, (uint32_t)indices.present};

    auto sameFamily = (indices.graphics == indices.present);
    createInfo.imageSharingMode = sameFamily ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = sameFamily ? 0 : queueFamilyIndices.size();
    createInfo.pQueueFamilyIndices = sameFamily ? nullptr : queueFamilyIndices.data();

    VkSwapchainKHR swapchain;
    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
        logger.error("magma.vulkan.swap-chain") << "Failed to create swap chain." << std::endl;
        exit(1);
    }
    m_swapchain = swapchain;

    // Retrieving image handles (we need to request the real image count as the implementation can require more)
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_images.data());

    // Saving some values
    m_extent = extent;
    m_imageFormat = surfaceFormat.format;
}

void Swapchain::createImageViews()
{
    m_imageViews.resize(m_images.size(), Capsule<VkImageView>{m_device.capsule(), vkDestroyImageView});

    for (uint32_t i = 0; i < m_images.size(); i++) {
        createImageView(m_device, m_images[i], m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_imageViews[i]);
    }
}
