
#include "./buffer.hpp"

#include "../helpers/device.hpp"

using namespace lava::magma;

void vulkan::createBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage,
                          vk::MemoryPropertyFlags properties, vk::UniqueBuffer& buffer, vk::UniqueDeviceMemory& bufferMemory)
{
    vk::BufferCreateInfo createInfo;
    createInfo.size = size;
    createInfo.usage = usage;
    createInfo.sharingMode = vk::SharingMode::eExclusive;

    auto bufferResult = device.createBufferUnique(createInfo);
    buffer = checkMove(bufferResult, "helpers.buffer", "Unable to create buffer.");

    vk::MemoryRequirements memRequirements;
    device.getBufferMemoryRequirements(buffer.get(), &memRequirements);

    vk::MemoryAllocateInfo allocInfo;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    auto bufferMemoryResult = device.allocateMemoryUnique(allocInfo);
    bufferMemory = checkMove(bufferMemoryResult, "helpers.buffer", "Unable to allocate buffer memory.");

    device.bindBufferMemory(buffer.get(), bufferMemory.get(), 0);
}

void vulkan::copyBuffer(vk::Device device, vk::Queue queue, vk::CommandPool commandPool, vk::Buffer srcBuffer,
                        vk::Buffer dstBuffer, vk::DeviceSize size, vk::DeviceSize offset)
{
    // Temporary command buffer
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    auto commandBufferResult = device.allocateCommandBuffersUnique(allocInfo);
    auto commandBuffer = std::move(vulkan::checkMove(commandBufferResult, "helpers.buffer", "Unable to create command buffer.")[0]);

    // Record
    vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

    commandBuffer->begin(&beginInfo);

    vk::BufferCopy copyRegion;
    copyRegion.size = size;
    copyRegion.srcOffset = offset;
    copyRegion.dstOffset = offset;
    commandBuffer->copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

    commandBuffer->end();

    // Make a fence
    vk::UniqueFence fence;
    vk::FenceCreateInfo createInfo;
    auto fenceResult = device.createFenceUnique(createInfo);
    fence = vulkan::checkMove(fenceResult, "helpers.buffer", "Unable to create fence.");

    // Execute it
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer.get();
    queue.submit(1, &submitInfo, fence.get());

    static const auto MAX = std::numeric_limits<uint64_t>::max();
    device.waitForFences(1u, &fence.get(), true, MAX);
}
