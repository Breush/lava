
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

    // Make a fence
    vulkan::Fence fence(device);
    vk::FenceCreateInfo fenceInfo;
    auto result = device.createFence(&fenceInfo, nullptr, fence.replace());
    if (result != vk::Result::eSuccess) {
        logger.error("magma.vulkan.helpers.buffer") << "Unable to create fence." << std::endl;
    }

    // Execute it
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    queue.submit(1, &submitInfo, fence.vk());

    static const auto MAX = std::numeric_limits<uint64_t>::max();
    device.waitForFences(1u, &fence, true, MAX);

    device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}
