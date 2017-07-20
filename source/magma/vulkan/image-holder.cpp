#include "./image-holder.hpp"

#include <lava/chamber/logger.hpp>

#include "./device.hpp"
#include "./image.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

ImageHolder::ImageHolder(vulkan::Device& device, vk::CommandPool& commandPool)
    : m_device(device)
    , m_commandPool(commandPool)
    , m_image{device.vk()}
    , m_memory{device.vk()}
    , m_view{device.vk()}
{
}

void ImageHolder::create(vk::Format format, vk::Extent2D extent, vk::ImageAspectFlagBits imageAspect)
{
    m_extent = extent;

    vk::ImageAspectFlags imageAspectFlags;
    vk::ImageUsageFlags imageUsageFlags;
    vk::MemoryPropertyFlags memoryPropertyFlags;
    vk::ImageLayout oldLayout;
    vk::ImageLayout newLayout;

    // Depth
    if (imageAspect == vk::ImageAspectFlagBits::eDepth) {
        imageAspectFlags = vk::ImageAspectFlagBits::eDepth;
        imageUsageFlags = vk::ImageUsageFlagBits::eDepthStencilAttachment;
        // @todo Following is useful because present uses an image. Try removing it, and see.
        // Better have this configurable in the function's interface
        imageUsageFlags |= vk::ImageUsageFlagBits::eSampled;
        memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
        oldLayout = vk::ImageLayout::eUndefined;
        newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }
    // Color
    else if (imageAspect == vk::ImageAspectFlagBits::eColor) {
        imageAspectFlags = vk::ImageAspectFlagBits::eColor;
        // @todo eColorAttachment is not always necessary... we should be able to control that
        imageUsageFlags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
        // @todo Following is useful because present uses an image. Try removing it, and see.
        imageUsageFlags |= vk::ImageUsageFlagBits::eSampled;
        memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        oldLayout = vk::ImageLayout::ePreinitialized;
        newLayout = vk::ImageLayout::eTransferDstOptimal;
    }
    else {
        logger.error("magma.vulkan.image-holder") << "Unknown image aspect for image holder. "
                                                  << "Valid ones are currently eDepth or eColor." << std::endl;
    }

    // @cleanup HPP

    vulkan::createImage(m_device, extent, format, vk::ImageTiling::eOptimal, imageUsageFlags,
                        vk::MemoryPropertyFlagBits::eDeviceLocal, m_image, m_memory);

    vulkan::createImageView(m_device, m_image, format, imageAspectFlags, m_view);

    vulkan::transitionImageLayout(m_device, reinterpret_cast<VkCommandPool&>(m_commandPool), reinterpret_cast<VkImage&>(m_image),
                                  reinterpret_cast<VkImageLayout&>(oldLayout), reinterpret_cast<VkImageLayout&>(newLayout));
}

void ImageHolder::copy(const void* data, vk::DeviceSize size)
{
    // @cleanup HPP
    const auto& vk_device = m_device.vk();

    //----- Staging buffer

    vulkan::Buffer stagingBuffer(vk_device);
    vulkan::DeviceMemory stagingBufferMemory(vk_device);

    vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    vulkan::createBuffer(m_device, size, usageFlags, propertyFlags, stagingBuffer, stagingBufferMemory);

    //----- Copy indeed

    void* targetData;
    vk::MemoryMapFlags memoryMapFlags;
    vk_device.mapMemory(stagingBufferMemory, 0, size, memoryMapFlags, &targetData);
    memcpy(targetData, data, size);
    vk_device.unmapMemory(stagingBufferMemory);

    vulkan::copyBufferToImage(m_device, m_commandPool, stagingBuffer, m_image, m_extent);
}

void ImageHolder::setup(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    if (channels != 4u) {
        logger.error("magma.vulkan.image-holder") << "Cannot handle image with " << static_cast<uint32_t>(channels)
                                                  << " channels. Only 4 is currently supported." << std::endl;
    }

    //----- Create

    vk::Extent2D extent = {width, height};
    create(vk::Format::eR8G8B8A8Unorm, extent, vk::ImageAspectFlagBits::eColor);

    //----- Copy

    copy(pixels.data(), width * height * channels);
}
