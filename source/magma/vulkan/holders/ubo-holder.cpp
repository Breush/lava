#include "./ubo-holder.hpp"

#include "../render-engine-impl.hpp"

using namespace lava::magma::vulkan;

UboHolder::UboHolder(const RenderEngine::Impl& engine)
    : m_engine(engine)
{
}

void UboHolder::init(vk::DescriptorSet descriptorSet, const std::vector<uint32_t>& uboSizes)
{
    const uint32_t uboCount = uboSizes.size();
    m_descriptorSet = descriptorSet;

    std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;
    std::vector<vk::WriteDescriptorSet> descriptorWrites;

    m_bufferHolders.reserve(uboCount);
    descriptorBufferInfos.resize(uboCount);
    descriptorWrites.resize(uboCount);

    for (auto i = 0u; i < uboCount; ++i) {
        m_bufferHolders.emplace_back(m_engine);
        auto& bufferHolder = m_bufferHolders[i];
        bufferHolder.create(vk::BufferUsageFlagBits::eUniformBuffer, uboSizes[i]);

        auto& descriptorBufferInfo = descriptorBufferInfos[i];
        descriptorBufferInfo.buffer = bufferHolder.buffer();
        descriptorBufferInfo.range = uboSizes[i];

        auto& descriptorWrite = descriptorWrites[i];
        descriptorWrite.dstSet = m_descriptorSet;
        descriptorWrite.dstBinding = i;
        descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &descriptorBufferInfo;
    }

    m_engine.device().updateDescriptorSets(descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

void UboHolder::copy(uint32_t bufferIndex, const void* data, vk::DeviceSize size)
{
    m_bufferHolders[bufferIndex].copy(data, size);
}
