#pragma once

#include <vulkan/vulkan.hpp>

namespace lava::magma::vulkan {
    /// Bind an image to the descriptor set.
    void updateDescriptorSet(vk::Device device, vk::DescriptorSet descriptorSet, vk::ImageView imageView, vk::Sampler sampler,
                             vk::ImageLayout imageLayout, uint32_t dstBinding, uint32_t dstArrayElement = 0u);
}
