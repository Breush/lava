#pragma once

#include <vulkan/vulkan.hpp>

namespace lava::magma::vulkan {
    /// Start a command.
    vk::CommandBuffer beginSingleTimeCommands(vk::Device device, vk::CommandPool commandPool);

    /// End a previously started command.
    void endSingleTimeCommands(vk::Device device, vk::Queue queue, vk::CommandPool commandPool, vk::CommandBuffer commandBuffer);
}
