#pragma once

#include <lava/magma/render-engine.hpp>

#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    class Device;
}

namespace lava::magma::vulkan {
    enum class BufferKind {
        Unknown,
        ShaderUniform, // UniformBuffer, staged memory
        ShaderStorage, // StorageBuffer, staged memory
        ShaderVertex,  // VertexBuffer, staged memory
        ShaderIndex,   // IndexBuffer, staged memory
    };

    /**
     * Simple wrapper around a vulkan Buffer,
     * holding its device memory and such.
     */
    class BufferHolder {
    public:
        BufferHolder() = delete;
        BufferHolder(const RenderEngine::Impl& engine);
        BufferHolder(const RenderEngine::Impl& engine, const std::string& name);

        /// Allocate all buffer memory.
        void create(BufferKind kind, vk::DeviceSize size);

        /// Copy data to the buffer.
        void copy(const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0u);

        /// Helper function to copy data to the buffer.
        template <class T>
        void copy(const T& data);


        vk::DeviceSize size() const { return m_size; }

    private:
        // References
        const RenderEngine::Impl& m_engine;
        std::string m_name;

        // Resources
        vulkan::Buffer m_stagingBuffer;
        vulkan::DeviceMemory m_stagingMemory;
        $attribute(vulkan::Buffer, buffer);
        $attribute(vulkan::DeviceMemory, memory);

        BufferKind m_kind = BufferKind::Unknown;
        vk::DeviceSize m_size = 0u;
    };
}

#include "buffer-holder.inl"
