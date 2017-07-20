#pragma once

#include <vulkan/vulkan.hpp>

#include "./device.hpp"

namespace lava::magma::vulkan {
    inline vk::CommandBuffer beginSingleTimeCommands(Device& device, vk::CommandPool commandPool)
    {
        vk::CommandBufferAllocateInfo bufferAllocateInfo{commandPool};
        bufferAllocateInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        device.vk().allocateCommandBuffers(&bufferAllocateInfo, &commandBuffer);
        // @todo cleanup HPP

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
        device.vk().freeCommandBuffers(commandPool, 1, &commandBuffer);
        // @todo cleanup HPP
    }
}
