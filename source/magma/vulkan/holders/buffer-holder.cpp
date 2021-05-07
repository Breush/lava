#include "./buffer-holder.hpp"

#include "../helpers/device.hpp"
#include "../render-engine-impl.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

namespace {
    void createBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage,
                      vk::MemoryPropertyFlags properties, vk::UniqueBuffer& buffer, vk::UniqueDeviceMemory& bufferMemory)
    {
        vk::BufferCreateInfo createInfo;
        createInfo.size = size;
        createInfo.usage = usage;
        createInfo.sharingMode = vk::SharingMode::eExclusive;

        auto bufferResult = device.createBufferUnique(createInfo);
        buffer = checkMove(bufferResult, "buffer-holder", "Unable to create buffer.");

        vk::MemoryRequirements memRequirements;
        device.getBufferMemoryRequirements(buffer.get(), &memRequirements);

        vk::MemoryAllocateInfo allocInfo;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

        vk::MemoryAllocateFlagsInfo allocFlagsInfo;
        if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
            allocFlagsInfo.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;
            allocInfo.pNext = &allocFlagsInfo;
        }

        auto bufferMemoryResult = device.allocateMemoryUnique(allocInfo);
        bufferMemory = checkMove(bufferMemoryResult, "buffer-holder", "Unable to allocate buffer memory.");

        device.bindBufferMemory(buffer.get(), bufferMemory.get(), 0);
    }

    void copyBuffer(vk::Device device, vk::Queue queue, vk::CommandPool commandPool, vk::Buffer srcBuffer,
                    vk::Buffer dstBuffer, vk::DeviceSize size, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset)
    {
        // Temporary command buffer
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        auto commandBufferResult = device.allocateCommandBuffersUnique(allocInfo);
        auto commandBuffer = std::move(checkMove(commandBufferResult, "buffer-holder", "Unable to create command buffer.")[0]);

        // Record
        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

        commandBuffer->begin(&beginInfo);

        vk::BufferCopy copyRegion;
        copyRegion.size = size;
        copyRegion.srcOffset = srcOffset;
        copyRegion.dstOffset = dstOffset;
        commandBuffer->copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

        commandBuffer->end();

        // Make a fence
        vk::UniqueFence fence;
        vk::FenceCreateInfo createInfo;
        auto fenceResult = device.createFenceUnique(createInfo);
        fence = checkMove(fenceResult, "buffer-holder", "Unable to create fence.");

        // Execute it
        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer.get();
        queue.submit(1, &submitInfo, fence.get());

        static const auto MAX = std::numeric_limits<uint64_t>::max();
        device.waitForFences(1u, &fence.get(), true, MAX);
    }
}

BufferHolder::BufferHolder(const RenderEngine::Impl& engine)
    : m_engine(engine)
{
}

BufferHolder::BufferHolder(const RenderEngine::Impl& engine, const std::string& name)
    : BufferHolder(engine)
{
    m_name = name;
}

void BufferHolder::create(BufferKind kind, BufferCpuIo cpuIo, vk::DeviceSize size)
{
    static const std::unordered_map<BufferKind, vk::BufferUsageFlags> kindToUsageFlagsMap({
        {BufferKind::Staging, vk::BufferUsageFlagBits::eTransferSrc},
        {BufferKind::StagingTarget, vk::BufferUsageFlagBits::eTransferDst},
        {BufferKind::ShaderUniform, vk::BufferUsageFlagBits::eUniformBuffer},
        {BufferKind::ShaderStorage, vk::BufferUsageFlagBits::eStorageBuffer},
        {BufferKind::ShaderVertex, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer}, // @fixme FOR RT compatibility
        {BufferKind::ShaderIndex, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eStorageBuffer}, // @fixme FOR RT compatibility
        {BufferKind::ShaderBindingTable, vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress},
        {BufferKind::AccelerationStructureInput, vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer}, // @fixme STorageBuffer FOR compatibility
        {BufferKind::AccelerationStructureStorage, vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress},
        {BufferKind::AccelerationStructureScratch, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress},
    });

    if (m_kind == kind && m_cpuIo == cpuIo && m_size == size) return;
    m_kind = kind;
    m_cpuIo = cpuIo;
    m_size = size;

    //----- Staging memory
    // @fixme Have choice: persistent staging memory or ONE-TIME use, so that we can free a bunch of memory usage!

    if (m_cpuIo == BufferCpuIo::PersistentStaging) {
        m_stagingBufferHolder = std::make_unique<BufferHolder>(m_engine, m_name + ".staging");
        m_stagingBufferHolder->create(BufferKind::Staging, BufferCpuIo::Direct, size);
    } else {
        m_stagingBufferHolder = nullptr;
    }

    //----- Final buffer

    vk::BufferUsageFlags usageFlags = kindToUsageFlagsMap.at(kind);
    if (m_cpuIo == BufferCpuIo::OnDemandStaging || m_cpuIo == BufferCpuIo::PersistentStaging) {
        usageFlags |= vk::BufferUsageFlagBits::eTransferDst;
    }

    vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
    if (m_cpuIo == BufferCpuIo::Direct) {
        propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    }

    createBuffer(m_engine.device(), m_engine.physicalDevice(), size, usageFlags, propertyFlags, m_buffer, m_memory);
    m_engine.deviceHolder().debugObjectName(m_memory.get(), m_name + ".buffer");
}

void BufferHolder::copy(const void* data, vk::DeviceSize size, vk::DeviceSize offset)
{
    if (m_cpuIo == BufferCpuIo::Direct) {
        auto targetData = map(size, offset);
        memcpy(targetData, data, size);
        unmap();
    }
    else if (m_cpuIo == BufferCpuIo::PersistentStaging) {
        m_stagingBufferHolder->copy(data, size, offset);
        copyBuffer(m_engine.device(), m_engine.transferQueue(), m_engine.transferCommandPool(), m_stagingBufferHolder->buffer(), m_buffer.get(), size, offset, offset);
    }
    else if (m_cpuIo == BufferCpuIo::OnDemandStaging) {
        BufferHolder stagingBufferHolder(m_engine, m_name + ".staging");
        stagingBufferHolder.create(BufferKind::Staging, BufferCpuIo::Direct, size);
        stagingBufferHolder.copy(data, size, 0u);
        copyBuffer(m_engine.device(), m_engine.transferQueue(), m_engine.transferCommandPool(), stagingBufferHolder.buffer(), m_buffer.get(), size, 0u, offset);
    }
    else {
        logger.error("magma.vulkan.buffer-holder") << "Buffer holder " << m_name << " cannot be a copied to because of its CpuIo." << std::endl;
    }
}

void* BufferHolder::map(vk::DeviceSize size, vk::DeviceSize offset)
{
    if (m_cpuIo != BufferCpuIo::Direct) {
        logger.error("magma.vulkan.buffer-holder") << "Buffer holder " << m_name << " cannot be a mapped to because of its CpuIo." << std::endl;
    }

    void* data;
    vk::MemoryMapFlags memoryMapFlags;
    m_engine.device().mapMemory(m_memory.get(), offset, size, memoryMapFlags, &data);
    return data;
}

void BufferHolder::unmap()
{
    m_engine.device().unmapMemory(m_memory.get());
}

vk::DeviceAddress BufferHolder::deviceAddress() const
{
    // @fixme Put in cache after first call!
    vk::BufferDeviceAddressInfo bufferDeviceAddressInfo;
    bufferDeviceAddressInfo.buffer = m_buffer.get();
    return m_engine.device().getBufferAddress(&bufferDeviceAddressInfo);
}
