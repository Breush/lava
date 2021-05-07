#include "./acceleration-structure-holder.hpp"

#include "../render-engine-impl.hpp"

#include "../helpers/command-buffer.hpp"

using namespace lava::magma::vulkan;

AccelerationStructureHolder::AccelerationStructureHolder(const RenderEngine::Impl& engine)
    : m_engine(engine)
    , m_bufferHolder(engine, "AccelerationStructureHolder")
{
}

void AccelerationStructureHolder::create(vk::AccelerationStructureTypeKHR type, vk::AccelerationStructureGeometryKHR geometry)
{
    auto device = m_engine.device();

    // --- Acceleration structure buffer

    vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
    geometryInfo.type = type;
    geometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
    geometryInfo.geometryCount = 1;
    geometryInfo.pGeometries = &geometry;

    vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo;
    device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, &geometryInfo,
                                                 &m_primitiveCount, &buildSizesInfo);

    m_bufferHolder.create(BufferKind::AccelerationStructureStorage, vulkan::BufferCpuIo::OnDemandStaging, buildSizesInfo.accelerationStructureSize);

    // --- Acceleration structure

    vk::AccelerationStructureCreateInfoKHR createInfo;
    createInfo.buffer = m_bufferHolder.buffer();
    createInfo.size = m_bufferHolder.size();
    createInfo.type = type;

    auto result = device.createAccelerationStructureKHRUnique(createInfo);
    m_accelerationStructure = checkMove(result, "acceleration-structure-holder", "Unable to create acceleration structure.");

    // --- Scratch buffer

    BufferHolder scratchBufferHolder(m_engine);
    auto scratchBufferSize = buildSizesInfo.buildScratchSize;
    scratchBufferHolder.create(BufferKind::AccelerationStructureScratch, vulkan::BufferCpuIo::None, scratchBufferSize);

    // --- Building

    vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo;
    buildGeometryInfo.type = type;
    buildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
    buildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
    buildGeometryInfo.dstAccelerationStructure = m_accelerationStructure.get();
    buildGeometryInfo.geometryCount = 1;
    buildGeometryInfo.pGeometries = &geometry;
    buildGeometryInfo.scratchData.deviceAddress = scratchBufferHolder.deviceAddress();

    vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo;
    buildRangeInfo.primitiveCount = m_primitiveCount;
    buildRangeInfo.primitiveOffset = 0;
    buildRangeInfo.firstVertex = 0;
    buildRangeInfo.transformOffset = 0;
    std::vector<vk::AccelerationStructureBuildRangeInfoKHR*> buildRangeInfos{ &buildRangeInfo };

    // @note This only works if accelerationStructureFeatures.accelerationStructureHostCommands is enabled,
    // but in my testing, my driver never supports it. So we need a command buffer below.
    // device.buildAccelerationStructuresKHR(nullptr, 1, &buildGeometryInfo, buildRangeInfos.data());

    auto commandPool = m_engine.commandPool();
    auto commandBuffer = beginSingleTimeCommands(device, commandPool);
    commandBuffer.buildAccelerationStructuresKHR(1, &buildGeometryInfo, buildRangeInfos.data());
    endSingleTimeCommands(device, m_engine.graphicsQueue(), commandPool, commandBuffer);
}

vk::DeviceAddress AccelerationStructureHolder::deviceAddress() const
{
    // @fixme Put in cache!
    vk::AccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo;
    accelerationDeviceAddressInfo.accelerationStructure = m_accelerationStructure.get();
    return m_engine.device().getAccelerationStructureAddressKHR(&accelerationDeviceAddressInfo);
}
