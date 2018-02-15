#include "./descriptor-holder.hpp"

#include <lava/chamber/logger.hpp>

#include "../render-engine-impl.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

DescriptorHolder::DescriptorHolder(const RenderEngine::Impl& engine)
    : m_engine(engine)
    , m_setLayout{engine.device()}
    , m_pool{engine.device()}
{
}

void DescriptorHolder::init(uint32_t maxSetCount, vk::ShaderStageFlags shaderStageFlags)
{
    //----- Set layout

    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings;
    uint32_t currentBinding = 0u;

    uint32_t storageBufferDescriptorCount = 0u;
    for (auto i = 0u; i < m_storageBufferSizes.size(); ++i) {
        vk::DescriptorSetLayoutBinding setLayoutBinding;
        setLayoutBinding.binding = currentBinding++;
        setLayoutBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
        setLayoutBinding.descriptorCount = m_storageBufferSizes[i];
        setLayoutBinding.stageFlags = shaderStageFlags;
        setLayoutBindings.emplace_back(setLayoutBinding);
        storageBufferDescriptorCount += m_storageBufferSizes[i];
    }

    uint32_t uniformBufferDescriptorCount = 0u;
    for (auto i = 0u; i < m_uniformBufferSizes.size(); ++i) {
        vk::DescriptorSetLayoutBinding setLayoutBinding;
        setLayoutBinding.binding = currentBinding++;
        setLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
        setLayoutBinding.descriptorCount = m_uniformBufferSizes[i];
        setLayoutBinding.stageFlags = shaderStageFlags;
        setLayoutBindings.emplace_back(setLayoutBinding);
        uniformBufferDescriptorCount += m_uniformBufferSizes[i];
    }

    uint32_t combinedImageSamplerDescriptorCount = 0u;
    for (auto i = 0u; i < m_combinedImageSamplerSizes.size(); ++i) {
        vk::DescriptorSetLayoutBinding setLayoutBinding;
        setLayoutBinding.binding = currentBinding++;
        setLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        setLayoutBinding.descriptorCount = m_combinedImageSamplerSizes[i];
        setLayoutBinding.stageFlags = shaderStageFlags;
        setLayoutBindings.emplace_back(setLayoutBinding);
        combinedImageSamplerDescriptorCount += m_combinedImageSamplerSizes[i];
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = setLayoutBindings.size();
    layoutInfo.pBindings = setLayoutBindings.data();

    if (m_engine.device().createDescriptorSetLayout(&layoutInfo, nullptr, m_setLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.descriptor-holder") << "Failed to create descriptor set layout." << std::endl;
    }

    //----- Pool

    std::vector<vk::DescriptorPoolSize> poolSizes;

    if (storageBufferDescriptorCount > 0u) {
        vk::DescriptorPoolSize poolSize;
        poolSize.type = vk::DescriptorType::eStorageBuffer;
        poolSize.descriptorCount = storageBufferDescriptorCount * maxSetCount;
        poolSizes.emplace_back(poolSize);
    }

    if (uniformBufferDescriptorCount > 0u) {
        vk::DescriptorPoolSize poolSize;
        poolSize.type = vk::DescriptorType::eUniformBuffer;
        poolSize.descriptorCount = uniformBufferDescriptorCount * maxSetCount;
        poolSizes.emplace_back(poolSize);
    }

    if (combinedImageSamplerDescriptorCount > 0u) {
        vk::DescriptorPoolSize poolSize;
        poolSize.type = vk::DescriptorType::eCombinedImageSampler;
        poolSize.descriptorCount = combinedImageSamplerDescriptorCount * maxSetCount;
        poolSizes.emplace_back(poolSize);
    }

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxSetCount;
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

    if (m_engine.device().createDescriptorPool(&poolInfo, nullptr, m_pool.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.descriptor-holder") << "Failed to create descriptor pool." << std::endl;
    }
}

vk::DescriptorSet DescriptorHolder::allocateSet(bool dummyBinding) const
{
    vk::DescriptorSet set;

    vk::DescriptorSetAllocateInfo allocateInfo;
    allocateInfo.descriptorPool = m_pool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &m_setLayout;

    if (m_engine.device().allocateDescriptorSets(&allocateInfo, &set) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.descriptor-holder") << "Failed to create descriptor set." << std::endl;
    }

    if (dummyBinding) {
        // Defaulting all combined image sampler bindings
        for (auto i = 0u; i < m_combinedImageSamplerSizes.size(); ++i) {
            updateDescriptorSet(m_engine.device(), set, m_engine.dummyImageView(), m_engine.dummySampler(),
                                vk::ImageLayout::eShaderReadOnlyOptimal, combinedImageSamplerBindingOffset() + i);
        }
    }

    return set;
}

void DescriptorHolder::freeSet(vk::DescriptorSet set) const
{
    if (m_pool.vk()) {
        m_engine.device().freeDescriptorSets(m_pool, 1, &set);
    }
}

void DescriptorHolder::updateSet(vk::DescriptorSet set, vk::Buffer buffer, vk::DeviceSize bufferSize, uint32_t storageBufferIndex)
{
    vk::DescriptorBufferInfo descriptorBufferInfo;
    descriptorBufferInfo.buffer = buffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = bufferSize;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = storageBufferBindingOffset() + storageBufferIndex;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
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
    descriptorWrite.dstBinding = combinedImageSamplerBindingOffset() + combinedImageSamplerIndex;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}
