
#include "./buffer.hpp"

#include "../helpers/device.hpp"

using namespace lava::chamber;
using namespace lava::magma;

void vulkan::createBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage,
                          vk::MemoryPropertyFlags properties, Buffer& buffer, DeviceMemory& bufferMemory)
{
    vk::BufferCreateInfo bufferInfo;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    if (device.createBuffer(&bufferInfo, nullptr, buffer.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.helpers.buffer") << "Failed to create buffer." << std::endl;
    }

    vk::MemoryRequirements memRequirements;
    device.getBufferMemoryRequirements(buffer, &memRequirements);

    vk::MemoryAllocateInfo allocInfo;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    if (device.allocateMemory(&allocInfo, nullptr, bufferMemory.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.helpers.buffer") << "Failed to allocate buffer memory." << std::endl;
    }

    device.bindBufferMemory(buffer, bufferMemory, 0);
}

void vulkan::copyBuffer(vk::Device device, vk::Queue queue, vk::CommandPool commandPool, const Buffer& srcBuffer,
                        const Buffer& dstBuffer, vk::DeviceSize size, vk::DeviceSize offset)
{
    // Temporary command buffer
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    device.allocateCommandBuffers(&allocInfo, &commandBuffer);

    // Record
    vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

    commandBuffer.begin(&beginInfo);

    vk::BufferCopy copyRegion;
    copyRegion.size = size;
    copyRegion.srcOffset = offset;
    copyRegion.dstOffset = offset;
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
