#include "./descriptor-holder.hpp"

#include "../render-engine-impl.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

namespace {
    static const std::unordered_map<DescriptorKind, vk::DescriptorType> kindToType{
        { DescriptorKind::AccelerationStructure, vk::DescriptorType::eAccelerationStructureKHR },
        { DescriptorKind::StorageBuffer, vk::DescriptorType::eStorageBuffer },
        { DescriptorKind::StorageImage, vk::DescriptorType::eStorageImage },
        { DescriptorKind::UniformBuffer, vk::DescriptorType::eUniformBuffer },
        { DescriptorKind::CombinedImageSampler, vk::DescriptorType::eCombinedImageSampler },
        { DescriptorKind::InputAttachment, vk::DescriptorType::eInputAttachment },
    };
}

DescriptorHolder::DescriptorHolder(const RenderEngine::Impl& engine)
    : m_engine(engine)
{
}

void DescriptorHolder::init(uint32_t maxSetCount, vk::ShaderStageFlags shaderStageFlags)
{
    //----- Set layout

    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings;

    for (auto& binding : m_bindings) {
        uint32_t currentBinding = binding.second.offset;
        auto descriptorType = kindToType.at(binding.first);

        binding.second.totalSize = 0u;
        for (auto size : binding.second.sizes) {
            vk::DescriptorSetLayoutBinding setLayoutBinding;
            setLayoutBinding.binding = currentBinding++;
            setLayoutBinding.descriptorType = descriptorType;
            setLayoutBinding.descriptorCount = size;
            setLayoutBinding.stageFlags = shaderStageFlags;
            setLayoutBindings.emplace_back(setLayoutBinding);
            binding.second.totalSize += size;
        }
    }

    vk::DescriptorSetLayoutCreateInfo setLayoutCreateInfo;
    setLayoutCreateInfo.bindingCount = setLayoutBindings.size();
    setLayoutCreateInfo.pBindings = setLayoutBindings.data();

    auto setLayoutResult = m_engine.device().createDescriptorSetLayoutUnique(setLayoutCreateInfo);
    m_setLayout = vulkan::checkMove(setLayoutResult, "descriptor-holder", "Unable to create descriptor set layout.");

    //----- Pool

    std::vector<vk::DescriptorPoolSize> poolSizes;

    for (auto& binding : m_bindings) {
        auto descriptorType = kindToType.at(binding.first);

        vk::DescriptorPoolSize poolSize;
        poolSize.type = descriptorType;
        poolSize.descriptorCount = binding.second.totalSize * maxSetCount;
        poolSizes.emplace_back(poolSize);
    }

    vk::DescriptorPoolCreateInfo poolCreateInfo;
    poolCreateInfo.poolSizeCount = poolSizes.size();
    poolCreateInfo.pPoolSizes = poolSizes.data();
    poolCreateInfo.maxSets = maxSetCount;
    poolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

    auto poolResult = m_engine.device().createDescriptorPoolUnique(poolCreateInfo);
    m_pool = vulkan::checkMove(poolResult, "descriptor-holder", "Unable to create descriptor pool.");
}

void DescriptorHolder::setSizes(DescriptorKind kind, const std::vector<uint32_t>& sizes)
{
    auto& binding = m_bindings[kind];
    binding.offset = m_nextOffset;
    binding.sizes = sizes;

    m_nextOffset += sizes.size();
}

vk::UniqueDescriptorSet DescriptorHolder::allocateSet(const std::string& debugName, bool dummyBinding) const
{
    vk::UniqueDescriptorSet set;

    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = m_pool.get();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_setLayout.get();

    auto result = m_engine.device().allocateDescriptorSetsUnique(allocInfo);
    set = std::move(checkMove(result, "descriptor-holder", "Unable to create descriptor set.")[0]);

    m_engine.deviceHolder().debugObjectName(set.get(), debugName);

    if (dummyBinding) {
        // Defaulting all combined image sampler bindings
        auto combinedImageSamplerBinding = m_bindings.at(DescriptorKind::CombinedImageSampler);
        for (auto i = 0u; i < combinedImageSamplerBinding.sizes.size(); ++i) {
            updateDescriptorSet(m_engine.device(), set.get(), m_engine.dummyImageView(), m_engine.dummySampler(),
                                vk::ImageLayout::eShaderReadOnlyOptimal, combinedImageSamplerBinding.offset + i);
        }
    }

    return set;
}

void DescriptorHolder::updateSet(vk::DescriptorSet set, vk::AccelerationStructureKHR accelerationStructure, uint32_t accelerationStructureIndex)
{
    vk::WriteDescriptorSetAccelerationStructureKHR descriptorWriteNext;
    descriptorWriteNext.accelerationStructureCount = 1u;
    descriptorWriteNext.pAccelerationStructures = &accelerationStructure;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.pNext = &descriptorWriteNext;
    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = m_bindings.at(DescriptorKind::AccelerationStructure).offset + accelerationStructureIndex;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eAccelerationStructureKHR;
    descriptorWrite.descriptorCount = 1;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void DescriptorHolder::updateSet(vk::DescriptorSet set, DescriptorKind kind, vk::Buffer buffer, vk::DeviceSize bufferSize, uint32_t index, uint32_t arrayIndex)
{
    vk::DescriptorBufferInfo descriptorBufferInfo;
    descriptorBufferInfo.buffer = buffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = bufferSize;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = m_bindings.at(kind).offset + index;
    descriptorWrite.dstArrayElement = arrayIndex;
    descriptorWrite.descriptorType = kindToType.at(kind);
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &descriptorBufferInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void DescriptorHolder::updateSet(vk::DescriptorSet set, vk::ImageView imageView, vk::Sampler sampler, vk::ImageLayout imageLayout,
                                 uint32_t combinedImageSamplerIndex)
{
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = m_bindings.at(DescriptorKind::CombinedImageSampler).offset + combinedImageSamplerIndex;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}

void DescriptorHolder::updateSet(vk::DescriptorSet set, DescriptorKind kind, vk::ImageView imageView, vk::ImageLayout imageLayout,
                                 uint32_t index)
{
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = imageView;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = m_bindings.at(kind).offset + index;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = kindToType.at(kind);
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}
