#include "./ubo-holder.hpp"

#include "../render-engine-impl.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

UboHolder::UboHolder(const RenderEngine::Impl& engine)
    : m_engine(engine)
{
}

UboHolder::UboHolder(const RenderEngine::Impl& engine, const std::string& name)
    : m_engine(engine)
{
    m_name = name;
}

void UboHolder::init(vk::DescriptorSet descriptorSet, uint32_t bindingOffset, const std::vector<UboSize>& uboSizes)
{
    m_descriptorSet = descriptorSet;
    m_bindingOffset = bindingOffset;

    const auto physicalDeviceProperties = m_engine.physicalDevice().getProperties();
    m_offsetAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;

    std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;
    std::vector<vk::WriteDescriptorSet> descriptorWrites;

    const uint32_t bufferCount = uboSizes.size();
    m_bufferHolders.reserve(bufferCount);

    uint32_t descriptorCount = 0u;
    for (const auto& uboSize : uboSizes) {
        descriptorCount += uboSize.count;
    }

    descriptorBufferInfos.reserve(descriptorCount);
    descriptorWrites.reserve(descriptorCount);

    for (auto bufferIndex = 0u; bufferIndex < bufferCount; ++bufferIndex) {
        const auto& uboSize = uboSizes[bufferIndex];

        vk::DeviceSize alignedSize = uboSize.size;
        if (uboSize.count > 1 && alignedSize % m_offsetAlignment) {
            alignedSize = (alignedSize / m_offsetAlignment + 1) * m_offsetAlignment;
        }

        m_bufferHolders.emplace_back(m_engine, m_name + ".ubo#" + std::to_string(bufferIndex));
        auto& bufferHolder = m_bufferHolders[bufferIndex];
        bufferHolder.create(vulkan::BufferKind::ShaderUniform, uboSize.count * alignedSize);

        for (auto arrayIndex = 0u; arrayIndex < uboSize.count; ++arrayIndex) {
            descriptorBufferInfos.emplace_back();
            auto& descriptorBufferInfo = descriptorBufferInfos.back();
            descriptorBufferInfo.buffer = bufferHolder.buffer();
            descriptorBufferInfo.offset = arrayIndex * alignedSize;
            descriptorBufferInfo.range = uboSize.size;

            descriptorWrites.emplace_back();
            auto& descriptorWrite = descriptorWrites.back();
            descriptorWrite.dstSet = m_descriptorSet;
            descriptorWrite.dstBinding = bindingOffset + bufferIndex;
            descriptorWrite.dstArrayElement = arrayIndex;
            descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &descriptorBufferInfo;
        }
    }

    m_engine.device().updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void UboHolder::copy(uint32_t bufferIndex, const void* data, vk::DeviceSize size, uint32_t arrayIndex)
{
    if (bufferIndex >= m_bufferHolders.size()) {
        logger.error("magma.vulkan.ubo-holder") << "Copying to buffer index " << bufferIndex << " but holds only "
                                                << m_bufferHolders.size() << " buffers." << std::endl;
    }

    // Realigning offset if needed
    vk::DeviceSize alignedSize = size;
    if (arrayIndex > 0 && alignedSize % m_offsetAlignment) {
        alignedSize = (alignedSize / m_offsetAlignment + 1) * m_offsetAlignment;
    }

    // @todo We could have unaligned data in the staging buffer, but have it aligned
    // to destination during the copy inside this call.
    m_bufferHolders[bufferIndex].copy(data, size, arrayIndex * alignedSize);
}
