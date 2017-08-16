#include "./command-buffer.hpp"

using namespace lava::magma;

vk::CommandBuffer vulkan::beginSingleTimeCommands(vk::Device device, vk::CommandPool commandPool)
{
    vk::CommandBufferAllocateInfo bufferAllocateInfo{commandPool};
    bufferAllocateInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    device.allocateCommandBuffers(&bufferAllocateInfo, &commandBuffer);

    vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    commandBuffer.begin(&beginInfo);

    return commandBuffer;
}

void vulkan::endSingleTimeCommands(vk::Device device, vk::Queue queue, vk::CommandPool commandPool,
                                   vk::CommandBuffer commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    queue.submit(1, &submitInfo, vk::Fence());

    queue.waitIdle();
    device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}
