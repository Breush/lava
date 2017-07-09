#include "./image-holder.hpp"

#include <lava/chamber/logger.hpp>

#include "./device.hpp"
#include "./image.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

ImageHolder::ImageHolder(vulkan::Device& device)
    : m_device(device)
    , m_image{device.vk()}
    , m_memory{device.vk()}
    , m_view{device.vk()}
{
}

void ImageHolder::create(vk::Format format, vk::Extent2D extent, vk::ImageAspectFlagBits imageAspect)
{
    vk::ImageAspectFlags imageAspectFlags;
    vk::ImageUsageFlags imageUsageFlags;
    vk::MemoryPropertyFlags memoryPropertyFlags;

    // Depth
    if (imageAspect == vk::ImageAspectFlagBits::eDepth) {
        imageAspectFlags = vk::ImageAspectFlagBits::eDepth;
        imageUsageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment;
        memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
    }
    // Color
    else if (imageAspect == vk::ImageAspectFlagBits::eColor) {
        imageAspectFlags = vk::ImageAspectFlagBits::eColor;
        imageUsageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
        memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    }
    else {
        logger.error("magma.vulkan.image-holder")
            << "Unknown image aspect for image holder. "
            << "Valid ones are currently vk::ImageAspectFlagBitseDepth or vk::ImageAspectFlagBitseColor." << std::endl;
    }

    // @cleanup HPP

    vulkan::createImage(m_device, extent, format, vk::ImageTiling::eOptimal, imageUsageFlags,
                        vk::MemoryPropertyFlagBits::eDeviceLocal, m_image, m_memory);

    vulkan::createImageView(m_device, m_image, format, imageAspectFlags, m_view);
}
