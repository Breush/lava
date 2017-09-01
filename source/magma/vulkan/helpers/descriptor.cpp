#include "./descriptor.hpp"

using namespace lava::magma;

void vulkan::updateDescriptorSet(vk::Device device, vk::DescriptorSet descriptorSet, vk::ImageView imageView, vk::Sampler sampler,
                                 vk::ImageLayout imageLayout, uint32_t dstBinding, uint32_t dstArrayElement)
{
    vk::DescriptorImageInfo imageInfo;
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = dstBinding;
    descriptorWrite.dstArrayElement = dstArrayElement;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = nullptr;
    descriptorWrite.pImageInfo = &imageInfo;
    descriptorWrite.pTexelBufferView = nullptr;

    device.updateDescriptorSets(1u, &descriptorWrite, 0, nullptr);
}
