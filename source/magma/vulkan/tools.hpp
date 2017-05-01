#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace lava::vulkan {
    /**
     * Convert a VkResult to a string.
     */
    std::string toString(VkResult errorCode)
    {
        switch (errorCode) {
#define STR(r)                                                                                                                                       \
    case VK_##r: return #r
            STR(NOT_READY);
            STR(TIMEOUT);
            STR(EVENT_SET);
            STR(EVENT_RESET);
            STR(INCOMPLETE);
            STR(ERROR_OUT_OF_HOST_MEMORY);
            STR(ERROR_OUT_OF_DEVICE_MEMORY);
            STR(ERROR_INITIALIZATION_FAILED);
            STR(ERROR_DEVICE_LOST);
            STR(ERROR_MEMORY_MAP_FAILED);
            STR(ERROR_LAYER_NOT_PRESENT);
            STR(ERROR_EXTENSION_NOT_PRESENT);
            STR(ERROR_FEATURE_NOT_PRESENT);
            STR(ERROR_INCOMPATIBLE_DRIVER);
            STR(ERROR_TOO_MANY_OBJECTS);
            STR(ERROR_FORMAT_NOT_SUPPORTED);
            STR(ERROR_SURFACE_LOST_KHR);
            STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
            STR(SUBOPTIMAL_KHR);
            STR(ERROR_OUT_OF_DATE_KHR);
            STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
            STR(ERROR_VALIDATION_FAILED_EXT);
            STR(ERROR_INVALID_SHADER_NV);
#undef STR
        default: return "UNKNOWN_ERROR";
        }
    }

    /**
     * Listing some support.
     */
    inline std::vector<VkExtensionProperties> availableExtensions()
    {
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> properties(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());
        return properties;
    }

    /**
     * Listing some support.
     */
    inline std::vector<VkLayerProperties> availableLayers()
    {
        uint32_t count = 0;
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        std::vector<VkLayerProperties> properties(count);
        vkEnumerateInstanceLayerProperties(&count, properties.data());
        return properties;
    }

    bool validationLayersSupported(const std::vector<const char*>& validationLayers)
    {
        auto layers = availableLayers();

        for (auto layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : layers) {
                if (strcmp(layerName, layerProperties.layerName)) continue;
                layerFound = true;
                break;
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }
}
