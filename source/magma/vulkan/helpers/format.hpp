#pragma once

#include <vulkan/vulkan.hpp>

namespace lava::magma::vulkan {
    vk::Format findSupportedFormat(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& candidates,
                                   vk::ImageTiling tiling, vk::FormatFeatureFlags features);

    vk::Format depthBufferFormat(vk::PhysicalDevice physicalDevice);
}
