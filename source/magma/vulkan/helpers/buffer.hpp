#pragma once

#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    /// Create a vulkan::Buffer and its DeviceMemory.
    void createBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage,
                      vk::MemoryPropertyFlags properties, Buffer& buffer, DeviceMemory& bufferMemory);

    /// Copy a buffer to an other one.
    void copyBuffer(vk::Device device, vk::Queue queue, vk::CommandPool commandPool, const Buffer& srcBuffer,
                    const Buffer& dstBuffer, vk::DeviceSize size);
}
