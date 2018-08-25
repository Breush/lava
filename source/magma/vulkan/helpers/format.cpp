#include "./format.hpp"

using namespace lava::magma;

vk::Format vulkan::findSupportedFormat(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& candidates,
                                       vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
    for (auto format : candidates) {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    return vk::Format::eUndefined;
}

vk::Format vulkan::depthBufferFormat(vk::PhysicalDevice physicalDevice)
{
    return vulkan::findSupportedFormat(physicalDevice,
                                       {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                       vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}
