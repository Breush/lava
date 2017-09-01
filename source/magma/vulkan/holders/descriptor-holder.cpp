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

void DescriptorHolder::init(const std::vector<uint32_t>& uniformBufferSizes,
                            const std::vector<uint32_t>& combinedImageSamplerSizes, uint32_t maxSetCount,
                            vk::ShaderStageFlags shaderStageFlags)
{
    //----- Set layout

    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings;
    uint32_t currentBinding = 0u;

    uint32_t uniformBufferDescriptorCount = 0u;
    for (auto i = 0u; i < uniformBufferSizes.size(); ++i) {
        vk::DescriptorSetLayoutBinding setLayoutBinding;
        setLayoutBinding.binding = currentBinding++;
        setLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
        setLayoutBinding.descriptorCount = uniformBufferSizes[i];
        setLayoutBinding.stageFlags = shaderStageFlags;
        setLayoutBindings.emplace_back(setLayoutBinding);
        uniformBufferDescriptorCount += uniformBufferSizes[i];
    }

    uint32_t combinedImageSamplerDescriptorCount = 0u;
    for (auto i = 0u; i < combinedImageSamplerSizes.size(); ++i) {
        vk::DescriptorSetLayoutBinding setLayoutBinding;
        setLayoutBinding.binding = currentBinding++;
        setLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        setLayoutBinding.descriptorCount = combinedImageSamplerSizes[i];
        setLayoutBinding.stageFlags = shaderStageFlags;
        setLayoutBindings.emplace_back(setLayoutBinding);
        combinedImageSamplerDescriptorCount += combinedImageSamplerSizes[i];
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = setLayoutBindings.size();
    layoutInfo.pBindings = setLayoutBindings.data();

    if (m_engine.device().createDescriptorSetLayout(&layoutInfo, nullptr, m_setLayout.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.descriptor-holder") << "Failed to create descriptor set layout." << std::endl;
    }

    //----- Pool

    std::vector<vk::DescriptorPoolSize> poolSizes;

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
