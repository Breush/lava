#pragma once

#include <lava/chamber/macros.hpp>

#include "./wrappers.hpp"

namespace lava::magma::vulkan {
    class Device;
}

namespace lava::magma::vulkan {
    /**
     * Simple wrapper around a vulkan Buffer,
     * holding its device memory and such.
     */
    class BufferHolder {
    public:
        BufferHolder() = delete;
        BufferHolder(Device& device, vk::CommandPool& commandPool);

        /// Allocate all buffer memory.
        void create(vk::BufferUsageFlagBits usage, vk::DeviceSize size);

        /// Copy data to the buffer.
        void copy(const void* data, vk::DeviceSize size);

        /// Helper function to copy data to the buffer.
        template <class T>
        void copy(const T& data);

    private:
        // References
        vulkan::Device& m_device;
        vk::CommandPool& m_commandPool; // @todo vulkan::CommandPool?

        // Resources
        vulkan::Buffer m_stagingBuffer;
        vulkan::DeviceMemory m_stagingMemory;
        $attribute(vulkan::Buffer, buffer);
        $attribute(vulkan::DeviceMemory, memory);
    };
}

#include "buffer-holder.inl"
