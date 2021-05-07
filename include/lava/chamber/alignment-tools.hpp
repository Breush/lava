#pragma once

#include <cstdint>

namespace lava::chamber {
    /// Return the value, but slightly grown to fit alignment requirements.
    // Taken from https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanTools.cpp
    inline uint32_t alignedSize(uint32_t value, uint32_t alignment)
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }
}
