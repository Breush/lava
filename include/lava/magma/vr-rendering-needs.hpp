#pragma once

#include <cstdint>

// Vulkan rendering backend forward definitions
struct VkPhysicalDevice_T;
using VkPhysicalDevice = VkPhysicalDevice_T*;

namespace lava::magma {
    enum class VrRenderingNeedsType {
        Vulkan,
    };

    struct VrRenderingNeedsInfo {
        VrRenderingNeedsType type;
        union {
            struct {
                VkPhysicalDevice physicalDevice;
            } vulkan;
        };
    };

    struct VrRenderingNeeds {
        union {
            struct {
                uint32_t instanceExtensionCount;
                const char* instanceExtensionsNames[16u];
                uint32_t deviceExtensionCount;
                const char* deviceExtensionsNames[16u];
            } vulkan;
        };
    };
}
