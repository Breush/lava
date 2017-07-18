#include "./buffer-holder.hpp"

#include <lava/chamber/logger.hpp>

#include "./device.hpp"
#include "./image.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

BufferHolder::BufferHolder(vulkan::Device& device, vk::CommandPool& commandPool)
    : m_device(device)
    , m_commandPool(commandPool)
    , m_stagingBuffer{device.vk()}
    , m_stagingMemory{device.vk()}
    , m_buffer{device.vk()}
    , m_memory{device.vk()}
{
}

void BufferHolder::create(vk::BufferUsageFlagBits usage, vk::DeviceSize size)
{
    //----- Staging memory

    vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    vulkan::createBuffer(m_device, size, usageFlags, propertyFlags, m_stagingBuffer, m_stagingMemory);

    //----- Final buffer

    usageFlags = vk::BufferUsageFlagBits::eTransferDst;
    propertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

    // Uniform Buffer
    if (usage == vk::BufferUsageFlagBits::eUniformBuffer) {
        usageFlags |= usage;
    }
    else {
        logger.error("magma.vulkan.buffer-holder") << "Unknown usage flag for buffer holder. "
                                                   << "Valid ones are currently eUniformBuffer." << std::endl;
    }

    vulkan::createBuffer(m_device, size, usageFlags, propertyFlags, m_buffer, m_memory);
}

void BufferHolder::copy(const void* data, vk::DeviceSize size)
{
    // @cleanup HPP
    const auto& vk_device = m_device.vk();

    // Copy to staging
    void* targetData;
    vk::MemoryMapFlags memoryMapFlags;
    vk_device.mapMemory(m_stagingMemory, 0, size, memoryMapFlags, &targetData);
    memcpy(targetData, data, size);
    vk_device.unmapMemory(m_stagingMemory);

    // And to final buffer
    // @todo This copyBuffer could probably be inlined here
    vulkan::copyBuffer(m_device, m_commandPool, m_stagingBuffer, m_buffer, size);
}

// @todo Make some ImageDescriptor/BufferDescriptor?
