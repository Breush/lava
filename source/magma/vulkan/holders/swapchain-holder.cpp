#include "./swapchain-holder.hpp"

#include "../helpers/queue.hpp"
#include "../helpers/swapchain.hpp"
#include "../render-engine-impl.hpp"

namespace {
    vk::SurfaceFormatKHR swapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
    {
        if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
            return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
        }

        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Unorm
                && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    vk::PresentModeKHR swapchainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
    {
        auto bestMode = vk::PresentModeKHR::eFifo;

        for (const auto& availablePresentMode : availablePresentModes) {
            // We prefer triple-buffering over other present modes
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
            else if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
                bestMode = availablePresentMode;
            }
        }

        return bestMode;
    }

    vk::Extent2D swapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const vk::Extent2D& windowExtent)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        vk::Extent2D actualExtent = windowExtent;
        actualExtent.width = std::min(capabilities.maxImageExtent.width, actualExtent.width);
        actualExtent.width = std::max(capabilities.minImageExtent.width, actualExtent.width);
        actualExtent.height = std::min(capabilities.maxImageExtent.height, actualExtent.height);
        actualExtent.height = std::max(capabilities.minImageExtent.height, actualExtent.height);

        return actualExtent;
    }
}

using namespace lava::magma::vulkan;
using namespace lava::chamber;

SwapchainHolder::SwapchainHolder(const RenderEngine::Impl& engine)
    : m_engine(engine)
{
}

void SwapchainHolder::init(vk::SurfaceKHR surface, const vk::Extent2D& windowExtent)
{
    createSwapchain(surface, windowExtent);
    createImageViews();
    createSemaphore();
}

void SwapchainHolder::recreate(vk::SurfaceKHR surface, const vk::Extent2D& windowExtent)
{
    m_engine.device().waitIdle();
    createSwapchain(surface, windowExtent);
    createImageViews();
}

vk::Result SwapchainHolder::acquireNextImage()
{
    static const auto MAX = std::numeric_limits<uint64_t>::max();

    return m_engine.device().acquireNextImageKHR(m_swapchain.get(), MAX, m_imageAvailableSemaphore.get(), nullptr, &m_currentIndex);
}

//----- Internal

void SwapchainHolder::createSwapchain(vk::SurfaceKHR surface, const vk::Extent2D& windowExtent)
{
    auto details = swapchainSupportDetails(m_engine.physicalDevice(), surface);

    auto surfaceFormat = swapchainSurfaceFormat(details.formats);
    auto presentMode = swapchainPresentMode(details.presentModes);
    auto extent = swapchainExtent(details.capabilities, windowExtent);

    uint32_t imagesCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imagesCount > details.capabilities.maxImageCount) {
        imagesCount = details.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = surface;
    createInfo.minImageCount = imagesCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = true;
    createInfo.oldSwapchain = m_swapchain.get();

    auto indices = findQueueFamilies(m_engine.physicalDevice(), &surface);
    std::vector<uint32_t> queueFamilyIndices = {(uint32_t)indices.graphics, (uint32_t)indices.present};

    auto sameFamily = (indices.graphics == indices.present);
    createInfo.imageSharingMode = sameFamily ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = sameFamily ? 0 : queueFamilyIndices.size();
    createInfo.pQueueFamilyIndices = sameFamily ? nullptr : queueFamilyIndices.data();

    // Can't replace directly: the oldSwapchain has to be valid
    auto result = m_engine.device().createSwapchainKHRUnique(createInfo);
    m_swapchain = vulkan::checkMove(result, "swapchain-holder", "Unable to create swapchain");

    // Retrieving image handles (we need to request the real image count as the implementation can require more)
    m_images = m_engine.device().getSwapchainImagesKHR(m_swapchain.get()).value;

    for (const auto& image : m_images) {
        m_engine.deviceHolder().debugObjectName(image, "magma.vulkan.swapchain-holder.image");
    }

    // Saving some values
    m_extent = extent;
    m_imageFormat = surfaceFormat.format;
}

void SwapchainHolder::createImageViews()
{
    m_imageViews.resize(m_images.size());

    for (uint32_t i = 0; i < m_images.size(); i++) {
        vk::ImageViewCreateInfo createInfo;
        createInfo.image = m_images[i];
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = m_imageFormat;
        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        auto result = m_engine.device().createImageViewUnique(createInfo);
        m_imageViews[i] = vulkan::checkMove(result, "swapchain-holder", "Unable to create image view.");

        m_engine.deviceHolder().debugObjectName(m_imageViews[i].get(), "swapchain." + std::to_string(i));
    }
}

void SwapchainHolder::createSemaphore()
{
    vk::SemaphoreCreateInfo createInfo;

    auto result = m_engine.device().createSemaphoreUnique(createInfo);
    m_imageAvailableSemaphore = vulkan::checkMove(result, "swapchain-holder", "Unable to create semaphore.");

    m_engine.deviceHolder().debugObjectName(m_imageAvailableSemaphore.get(), "swapchain-holder.image-available");
}
