#pragma once

#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    /// Create a UniqueBuffer and its UniqueDeviceMemory.
    void createBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage,
                      vk::MemoryPropertyFlags properties, vk::UniqueBuffer& buffer, vk::UniqueDeviceMemory& bufferMemory);

    /// Copy a buffer to an other one. @fixme Useful to a helper? This is not really shared...
    void copyBuffer(vk::Device device, vk::Queue queue, vk::CommandPool commandPool, vk::Buffer srcBuffer,
                    vk::Buffer dstBuffer, vk::DeviceSize size, vk::DeviceSize offset = 0u);
}
