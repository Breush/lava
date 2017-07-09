#pragma once

#include <lava/chamber/logger.hpp>

#include "./device.hpp"
#include "./tools.hpp"
#include "./wrappers.hpp"

// @cleanup HPP Remove unused old fashion functions

namespace lava::magma::vulkan {
    inline void createBuffer(Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                             Buffer& buffer, DeviceMemory& bufferMemory)
    {
        const auto& vk_device = device.vk(); // @cleanup HPP

        vk::BufferCreateInfo bufferInfo;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        if (vk_device.createBuffer(&bufferInfo, nullptr, buffer.replace()) != vk::Result::eSuccess) {
            chamber::logger.error("magma.vulkan.buffer") << "Failed to create buffer." << std::endl;
        }

        vk::MemoryRequirements memRequirements;
        vk_device.getBufferMemoryRequirements(buffer, &memRequirements);

        vk::MemoryAllocateInfo allocInfo;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(device.physicalDevice(), memRequirements.memoryTypeBits,
                                                   reinterpret_cast<VkMemoryPropertyFlags&>(properties));

        if (vk_device.allocateMemory(&allocInfo, nullptr, bufferMemory.replace()) != vk::Result::eSuccess) {
            chamber::logger.error("magma.vulkan.buffer") << "Failed to allocate buffer memory." << std::endl;
        }

        vk_device.bindBufferMemory(buffer, bufferMemory, 0);
    }

    inline void copyBuffer(Device& device, const vk::CommandPool& commandPool, const Buffer& srcBuffer, const Buffer& dstBuffer,
                           vk::DeviceSize size)
    {
        const auto& vk_device = device.vk(); // @cleanup HPP

        // Temporary command buffer
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        vk_device.allocateCommandBuffers(&allocInfo, &commandBuffer);

        // Record
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        commandBuffer.begin(&beginInfo);

        vk::BufferCopy copyRegion;
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

        commandBuffer.end();

        // Execute it
        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // @cleanup HPP graphicsQueue is old API
        vkQueueSubmit(device.graphicsQueue(), 1, &reinterpret_cast<VkSubmitInfo&>(submitInfo), VK_NULL_HANDLE);
        vkQueueWaitIdle(device.graphicsQueue());

        vk_device.freeCommandBuffers(commandPool, 1, &commandBuffer);
    }
}

namespace lava::magma::vulkan {
    // @todo Move to cpp
    inline void createBuffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                             Capsule<VkBuffer>& buffer, Capsule<VkDeviceMemory>& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, buffer.replace()) != VK_SUCCESS) {
            chamber::logger.error("magma.vulkan.buffer") << "Failed to create buffer." << std::endl;
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(device.physicalDevice(), memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, bufferMemory.replace()) != VK_SUCCESS) {
            chamber::logger.error("magma.vulkan.buffer") << "Failed to allocate buffer memory." << std::endl;
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    inline void copyBuffer(Device& device, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        // Temporary command buffer
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        // Record
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        // Execute it
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device.graphicsQueue());

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

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
