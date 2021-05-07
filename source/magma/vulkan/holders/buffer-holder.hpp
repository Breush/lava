#pragma once

#include <lava/magma/render-engine.hpp>

#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    enum class BufferCpuIo {
        Unknown,
        None,               // Should never be read/written from CPU
        Direct,             // Manipulated directly
        OnDemandStaging,    // Manipulated through a staging buffer created each time a map is needed
        PersistentStaging,  // Manipulated through a persistent staging buffer
    };

    enum class BufferKind {
        Unknown,
        Staging,            // TransferSrc
        StagingTarget,      // TransferDst
        ShaderUniform,      // UniformBuffer
        ShaderStorage,      // StorageBuffer
        ShaderVertex,       // VertexBuffer
        ShaderIndex,        // IndexBuffer
        ShaderBindingTable, // ShaderBindingTableKHR
        AccelerationStructureInput,   // AccelerationStructureBuildInputReadOnlyKHR
        AccelerationStructureStorage, // AccelerationStructureStorageKHR
        AccelerationStructureScratch, // StorageBuffer
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
        void create(BufferKind kind, BufferCpuIo cpuIo, vk::DeviceSize size);

        /// Copy data to the buffer.
        void copy(const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0u);

        /// Helper function to copy data to the buffer.
        template <class T>
        void copy(const T& data);

        /// To be used only with BufferCpuIo::Direct buffers
        void* map(vk::DeviceSize size, vk::DeviceSize offset = 0u);
        void unmap();

        /// Get the buffer device address.
        vk::DeviceAddress deviceAddress() const;

        const vk::DeviceMemory& memory() const { return m_memory.get(); }
        const vk::Buffer& buffer() const { return m_buffer.get(); }
        vk::DeviceSize size() const { return m_size; }

    private:
        // References
        const RenderEngine::Impl& m_engine;
        std::string m_name;

        // Resources
        vk::UniqueBuffer m_buffer;
        vk::UniqueDeviceMemory m_memory;
        std::unique_ptr<BufferHolder> m_stagingBufferHolder;

        BufferKind m_kind = BufferKind::Unknown;
        BufferCpuIo m_cpuIo = BufferCpuIo::Unknown;
        vk::DeviceSize m_size = 0u;
    };
}

#include "buffer-holder.inl"
