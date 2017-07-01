#pragma once

#include <vulkan/vulkan.hpp>

#include "./device.hpp"

namespace lava::magma::vulkan {
    inline vk::CommandBuffer beginSingleTimeCommands(Device& device, vk::CommandPool commandPool)
    {
        vk::CommandBufferAllocateInfo bufferAllocateInfo{commandPool};
        bufferAllocateInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        vk::Device(device).allocateCommandBuffers(&bufferAllocateInfo, &commandBuffer);
        // @todo Cast clean

        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
        commandBuffer.begin(&beginInfo);

        return commandBuffer;
    }

    inline void endSingleTimeCommands(Device& device, vk::CommandPool commandPool, vk::CommandBuffer commandBuffer)
    {
        commandBuffer.end();

        vk::SubmitInfo submitInfo = {};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        vk::Queue(device.graphicsQueue()).submit(1, &submitInfo, vk::Fence());

        vk::Queue(device.graphicsQueue()).waitIdle();
        vk::Device(device).freeCommandBuffers(commandPool, 1, &commandBuffer);
        // @todo Cast clean
    }
}
