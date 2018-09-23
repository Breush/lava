#pragma once

namespace lava::magma::vulkan {
    /// Find a valid memory type for the physical device.
    uint32_t findMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
}
