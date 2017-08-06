#pragma once

#include <lava/chamber/logger.hpp>

#include "./tools.hpp"
#include "./wrappers.hpp"

// @todo Move to cpp
namespace lava::magma::vulkan {
    inline void createBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DeviceSize size,
                             vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, Buffer& buffer,
                             DeviceMemory& bufferMemory)
    {
        vk::BufferCreateInfo bufferInfo;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        if (device.createBuffer(&bufferInfo, nullptr, buffer.replace()) != vk::Result::eSuccess) {
            chamber::logger.error("magma.vulkan.buffer") << "Failed to create buffer." << std::endl;
        }

        vk::MemoryRequirements memRequirements;
        device.getBufferMemoryRequirements(buffer, &memRequirements);

        vk::MemoryAllocateInfo allocInfo;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits,
                                                   reinterpret_cast<VkMemoryPropertyFlags&>(properties)); // @cleanup HPP

        if (device.allocateMemory(&allocInfo, nullptr, bufferMemory.replace()) != vk::Result::eSuccess) {
            chamber::logger.error("magma.vulkan.buffer") << "Failed to allocate buffer memory." << std::endl;
        }

        device.bindBufferMemory(buffer, bufferMemory, 0);
    }

    inline void copyBuffer(vk::Device device, vk::Queue queue, vk::CommandPool commandPool, const Buffer& srcBuffer,
                           const Buffer& dstBuffer, vk::DeviceSize size)
    {
        // Temporary command buffer
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        device.allocateCommandBuffers(&allocInfo, &commandBuffer);

        // Record
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        commandBuffer.begin(&beginInfo);

        vk::BufferCopy copyRegion;
        copyRegion.size = size;
        commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

        commandBuffer.end();

        // Execute it
        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        queue.submit(1, &submitInfo, nullptr);

        queue.waitIdle();
        device.freeCommandBuffers(commandPool, 1, &commandBuffer);
    }
}

// @cleanup HPP Remove
namespace lava::magma::vulkan {
    inline VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates,
                                        VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        chamber::logger.error("magma.vulkan.buffer") << "Unable to find valid format." << std::endl;
        return VK_FORMAT_UNDEFINED;
    }

    inline VkFormat findDepthBufferFormat(VkPhysicalDevice physicalDevice)
    {
        return findSupportedFormat(physicalDevice,
                                   {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                   VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}
