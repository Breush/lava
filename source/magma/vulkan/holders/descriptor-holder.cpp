#include "./descriptor-holder.hpp"

#include "../render-engine-impl.hpp"

using namespace lava::magma::vulkan;
using namespace lava::chamber;

DescriptorHolder::DescriptorHolder(const RenderEngine::Impl& engine)
    : m_engine(engine)
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

    uint32_t inputAttachmentDescriptorCount = 0u;
    for (auto i = 0u; i < m_inputAttachmentSizes.size(); ++i) {
        vk::DescriptorSetLayoutBinding setLayoutBinding;
        setLayoutBinding.binding = currentBinding++;
        setLayoutBinding.descriptorType = vk::DescriptorType::eInputAttachment;
        setLayoutBinding.descriptorCount = m_inputAttachmentSizes[i];
        setLayoutBinding.stageFlags = shaderStageFlags;
        setLayoutBindings.emplace_back(setLayoutBinding);
        inputAttachmentDescriptorCount += m_inputAttachmentSizes[i];
    }

    vk::DescriptorSetLayoutCreateInfo setLayoutCreateInfo;
    setLayoutCreateInfo.bindingCount = setLayoutBindings.size();
    setLayoutCreateInfo.pBindings = setLayoutBindings.data();

    auto setLayoutResult = m_engine.device().createDescriptorSetLayoutUnique(setLayoutCreateInfo);
    m_setLayout = vulkan::checkMove(setLayoutResult, "descriptor-holder", "Unable to create descriptor set layout.");

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

    if (inputAttachmentDescriptorCount > 0u) {
        vk::DescriptorPoolSize poolSize;
        poolSize.type = vk::DescriptorType::eInputAttachment;
        poolSize.descriptorCount = inputAttachmentDescriptorCount * maxSetCount;
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
        for (auto i = 0u; i < m_combinedImageSamplerSizes.size(); ++i) {
            updateDescriptorSet(m_engine.device(), set.get(), m_engine.dummyImageView(), m_engine.dummySampler(),
                                vk::ImageLayout::eShaderReadOnlyOptimal, combinedImageSamplerBindingOffset() + i);
        }
    }

    return set;
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

void DescriptorHolder::updateSet(vk::DescriptorSet set, vk::ImageView imageView, vk::ImageLayout imageLayout,
                                 uint32_t inputAttachmentIndex)
{
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = imageView;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = set;
    descriptorWrite.dstBinding = inputAttachmentBindingOffset() + inputAttachmentIndex;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eInputAttachment;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    m_engine.device().updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}
