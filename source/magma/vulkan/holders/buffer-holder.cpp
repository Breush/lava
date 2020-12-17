#include "./buffer-holder.hpp"

#include "../helpers/buffer.hpp"
#include "../render-engine-impl.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

BufferHolder::BufferHolder(const RenderEngine::Impl& engine)
    : m_engine(engine)
{
}

BufferHolder::BufferHolder(const RenderEngine::Impl& engine, const std::string& name)
    : BufferHolder(engine)
{
    m_name = name;
}

void BufferHolder::create(BufferKind kind, vk::DeviceSize size)
{
    static const std::unordered_map<BufferKind, vk::BufferUsageFlags> kindToUsageFlagsMap({
        {BufferKind::ShaderUniform, vk::BufferUsageFlagBits::eUniformBuffer},
        {BufferKind::ShaderStorage, vk::BufferUsageFlagBits::eStorageBuffer},
        {BufferKind::ShaderVertex, vk::BufferUsageFlagBits::eVertexBuffer},
        {BufferKind::ShaderIndex, vk::BufferUsageFlagBits::eIndexBuffer}
    });

    if (m_kind == kind && m_size == size) return;
    m_kind = kind;
    m_size = size;

    //----- Staging memory

    bool needStagingMemory = true;

    if (needStagingMemory) {
    vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    vulkan::createBuffer(m_engine.device(), m_engine.physicalDevice(), size, usageFlags, propertyFlags, m_stagingBuffer,
                         m_stagingMemory);
    }

    //----- Final buffer

    vk::BufferUsageFlags usageFlags = kindToUsageFlagsMap.at(kind);
    vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

    if (needStagingMemory) {
        usageFlags |= vk::BufferUsageFlagBits::eTransferDst;
    }

    vulkan::createBuffer(m_engine.device(), m_engine.physicalDevice(), size, usageFlags, propertyFlags, m_buffer, m_memory);
    m_engine.deviceHolder().debugObjectName(m_memory.get(), m_name + ".buffer");
}

void BufferHolder::copy(const void* data, vk::DeviceSize size, vk::DeviceSize offset)
{
    // Copy to staging
    void* targetData;
    vk::MemoryMapFlags memoryMapFlags;
    m_engine.device().mapMemory(m_stagingMemory.get(), offset, size, memoryMapFlags, &targetData);
    memcpy(targetData, data, size);
    m_engine.device().unmapMemory(m_stagingMemory.get());

    // And to final buffer
    vulkan::copyBuffer(m_engine.device(), m_engine.transferQueue(), m_engine.transferCommandPool(), m_stagingBuffer.get(), m_buffer.get(),
                       size, offset);
}
