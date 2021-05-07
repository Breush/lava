#pragma once

#include "./buffer-holder.hpp"

namespace lava::magma::vulkan {
    /**
     * Simple wrapper around a vulkan Acceleration Structure (ray tracing extension).
     */
    class AccelerationStructureHolder {
    public:
        AccelerationStructureHolder() = delete;
        AccelerationStructureHolder(const RenderEngine::Impl& engine);

        uint32_t primitiveCount() const { return m_primitiveCount; }
        void primitiveCount(uint32_t primitiveCount) { m_primitiveCount = primitiveCount; }

        // @fixme COMMENT
        void create(vk::AccelerationStructureTypeKHR type,
                    vk::AccelerationStructureGeometryKHR geometry);

        vk::AccelerationStructureKHR accelerationStructure() const { return m_accelerationStructure.get(); }
        vk::Buffer buffer() const { return m_bufferHolder.buffer(); }
        vk::DeviceSize size() const { return m_bufferHolder.size(); }
        vk::DeviceAddress deviceAddress() const;

    private:
        // References
        const RenderEngine::Impl& m_engine;

        // Configuration
        uint32_t m_primitiveCount = 1u;

        // Resources
        vk::UniqueAccelerationStructureKHR m_accelerationStructure;
        vulkan::BufferHolder m_bufferHolder;
    };
}
