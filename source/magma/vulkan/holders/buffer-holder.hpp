#pragma once

#include <lava/magma/render-engine.hpp>

#include "../wrappers.hpp"

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
        BufferHolder(const RenderEngine::Impl& engine);

        /// Allocate all buffer memory.
        void create(vk::BufferUsageFlagBits usage, vk::DeviceSize size);

        /// Copy data to the buffer.
        void copy(const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0u);

        /// Helper function to copy data to the buffer.
        template <class T>
        void copy(const T& data);

    private:
        // References
        const RenderEngine::Impl& m_engine;

        // Resources
        vulkan::Buffer m_stagingBuffer;
        vulkan::DeviceMemory m_stagingMemory;
        $attribute(vulkan::Buffer, buffer);
        $attribute(vulkan::DeviceMemory, memory);
    };
}

#include "buffer-holder.inl"
