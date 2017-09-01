#include "./buffer-holder.hpp"

#include <lava/chamber/logger.hpp>

#include "../helpers/buffer.hpp"
#include "../render-engine-impl.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

BufferHolder::BufferHolder(const RenderEngine::Impl& engine)
    : m_engine(engine)
    , m_stagingBuffer{engine.device()}
    , m_stagingMemory{engine.device()}
    , m_buffer{engine.device()}
    , m_memory{engine.device()}
{
}

void BufferHolder::create(vk::BufferUsageFlagBits usage, vk::DeviceSize size)
{
    if (usage != vk::BufferUsageFlagBits::eUniformBuffer && usage != vk::BufferUsageFlagBits::eVertexBuffer
        && usage != vk::BufferUsageFlagBits::eIndexBuffer) {
        logger.error("magma.vulkan.buffer-holder")
            << "Unknown usage flag for buffer holder: " << vk::to_string(usage) << ". "
            << "Valid ones are currently UniformBuffer, VertexBuffer and IndexBuffer." << std::endl;
    }

    //----- Staging memory

    vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    vulkan::createBuffer(m_engine.device(), m_engine.physicalDevice(), size, usageFlags, propertyFlags, m_stagingBuffer,
                         m_stagingMemory);

    //----- Final buffer

    usageFlags = vk::BufferUsageFlagBits::eTransferDst;
    propertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

    // Uniform Buffer
    usageFlags |= usage;

    vulkan::createBuffer(m_engine.device(), m_engine.physicalDevice(), size, usageFlags, propertyFlags, m_buffer, m_memory);
}

void BufferHolder::copy(const void* data, vk::DeviceSize size, vk::DeviceSize offset)
{
    // Copy to staging
    void* targetData;
    vk::MemoryMapFlags memoryMapFlags;
    m_engine.device().mapMemory(m_stagingMemory, offset, size, memoryMapFlags, &targetData);
    memcpy(targetData, data, size);
    m_engine.device().unmapMemory(m_stagingMemory);

    // And to final buffer
    vulkan::copyBuffer(m_engine.device(), m_engine.graphicsQueue(), m_engine.commandPool(), m_stagingBuffer, m_buffer, size,
                       offset);
}

// @todo Make some ImageDescriptor/BufferDescriptor?
