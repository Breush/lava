#include "./buffer-holder.hpp"

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

void DescriptorHolder::init(uint32_t uniformBufferCount, uint32_t combinedImageSamplerCount, uint32_t maxSetCount,
                            vk::ShaderStageFlags shaderStageFlags)
{
    //----- Set layout

    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings;
    uint32_t currentBinding = 0u;

    for (auto i = 0u; i < uniformBufferCount; ++i) {
        vk::DescriptorSetLayoutBinding setLayoutBinding;
        setLayoutBinding.binding = currentBinding++;
        setLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
        setLayoutBinding.descriptorCount = 1;
        setLayoutBinding.stageFlags = shaderStageFlags;
        setLayoutBindings.emplace_back(setLayoutBinding);
    }

    for (auto i = 0u; i < combinedImageSamplerCount; ++i) {
        vk::DescriptorSetLayoutBinding setLayoutBinding;
        setLayoutBinding.binding = currentBinding++;
        setLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        setLayoutBinding.descriptorCount = 1;
        setLayoutBinding.stageFlags = shaderStageFlags;
        setLayoutBindings.emplace_back(setLayoutBinding);
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = setLayoutBindings.size();
    layoutInfo.pBindings = setLayoutBindings.data();

    if (m_engine.device().createDescriptorSetLayout(&layoutInfo, nullptr, m_setLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.descriptor-holder") << "Failed to create descriptor set layout." << std::endl;
    }

    //----- Pool

    std::vector<vk::DescriptorPoolSize> poolSizes;

    if (uniformBufferCount > 0u) {
        vk::DescriptorPoolSize poolSize;
        poolSize.type = vk::DescriptorType::eUniformBuffer;
        poolSize.descriptorCount = uniformBufferCount * maxSetCount;
        poolSizes.emplace_back(poolSize);
    }

    if (combinedImageSamplerCount > 0u) {
        vk::DescriptorPoolSize poolSize;
        poolSize.type = vk::DescriptorType::eCombinedImageSampler;
        poolSize.descriptorCount = combinedImageSamplerCount * maxSetCount;
        poolSizes.emplace_back(poolSize);
    }

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxSetCount;

    if (m_engine.device().createDescriptorPool(&poolInfo, nullptr, m_pool.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.descriptor-holder") << "Failed to create descriptor pool." << std::endl;
    }
}

vk::DescriptorSet DescriptorHolder::allocateSet() const
{
    vk::DescriptorSet set;

    vk::DescriptorSetAllocateInfo allocateInfo;
    allocateInfo.descriptorPool = m_pool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &m_setLayout;

    if (m_engine.device().allocateDescriptorSets(&allocateInfo, &set) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.descriptor-holder") << "Failed to create descriptor set." << std::endl;
    }

    return set;
}
