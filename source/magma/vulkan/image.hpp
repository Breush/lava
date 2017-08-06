#pragma once

#include <lava/chamber/logger.hpp>
#include <vulkan/vulkan.hpp>

#include "./buffer.hpp"
#include "./command-buffer.hpp"
#include "./wrappers.hpp"

// @todo This should probably be moved to ImageHolder, if it is the only one using that

namespace lava::magma::vulkan {
    inline void createImage(vk::Device device, vk::PhysicalDevice physicalDevice, const vk::Extent2D& extent, vk::Format format,
                            vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, Image& image,
                            DeviceMemory& imageMemory)
    {
        vk::ImageCreateInfo imageInfo;
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent.width = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = vk::ImageLayout::ePreinitialized;
        imageInfo.usage = usage;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;

        if (device.createImage(&imageInfo, nullptr, image.replace()) != vk::Result::eSuccess) {
            chamber::logger.error("magma.vulkan.image") << "Failed to create image." << std::endl;
        }

        // Memory
        vk::MemoryRequirements memRequirements;
        device.getImageMemoryRequirements(image, &memRequirements);

        vk::MemoryAllocateInfo allocInfo;
        allocInfo.allocationSize = memRequirements.size;
        // @cleanup HPP
        allocInfo.memoryTypeIndex =
            findMemoryType(reinterpret_cast<VkPhysicalDevice&>(physicalDevice), memRequirements.memoryTypeBits,
                           reinterpret_cast<VkMemoryPropertyFlags&>(properties));

        if (device.allocateMemory(&allocInfo, nullptr, imageMemory.replace()) != vk::Result::eSuccess) {
            chamber::logger.error("magma.vulkan.image") << "Failed to allocate image memory." << std::endl;
        }

        // @fixme Do we /really/ want to bind all image memories?
        device.bindImageMemory(image, imageMemory, 0);
    }

    inline void createImageView(vk::Device device, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags,
                                ImageView& imageView)
    {
        vk::ImageViewCreateInfo viewInfo;
        viewInfo.image = image;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (device.createImageView(&viewInfo, nullptr, imageView.replace()) != vk::Result::eSuccess) {
            chamber::logger.error("magma.vulkan.image") << "Failed to create image view." << std::endl;
        }
    }

    inline void copyBufferToImage(vk::Device device, vk::Queue graphicsQueue, vk::CommandPool commandPool, vk::Buffer buffer,
                                  vk::Image image, const vk::Extent2D& extent)
    {
        auto commandBuffer = beginSingleTimeCommands(device, commandPool);

        vk::BufferImageCopy region;
        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = vk::Extent3D{extent.width, extent.height, 1};

        commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

        endSingleTimeCommands(device, graphicsQueue, commandPool, commandBuffer);
    }

    inline void transitionImageLayout(vk::Device device, vk::Queue graphicsQueue, vk::CommandPool commandPool, vk::Image image,
                                      vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
    {
        auto commandBuffer = beginSingleTimeCommands(device, commandPool);

        vk::ImageMemoryBarrier barrier;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.image = image;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;

        if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        }
        else {
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        }

        if (oldLayout == vk::ImageLayout::ePreinitialized && newLayout == vk::ImageLayout::eTransferSrcOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        }
        else if (oldLayout == vk::ImageLayout::ePreinitialized && newLayout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        }
        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        }
        else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
            barrier.dstAccessMask =
                vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        }
        else {
            chamber::logger.error("magma.vulkan.image") << "Unsupported layout transition." << std::endl;
        }

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe,
                                      vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

        endSingleTimeCommands(device, graphicsQueue, commandPool, commandBuffer);
    }
}
