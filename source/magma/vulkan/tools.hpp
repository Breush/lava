#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace lava::vulkan {

    std::string toString(VkDebugReportObjectTypeEXT objType)
    {
        switch (objType) {
        case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT: return "instance";
        case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT: return "physical-device";
        case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT: return "device";
        case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT: return "queue";
        case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT: return "semaphore";
        case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT: return "command-buffer";
        case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT: return "fence";
        case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT: return "device-memory";
        case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT: return "buffer";
        case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT: return "image";
        case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT: return "event";
        case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT: return "query-pool";
        case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT: return "buffer-view";
        case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT: return "image-view";
        case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT: return "shader-module";
        case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT: return "pipeline-cache";
        case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT: return "pipeline-layout";
        case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT: return "render-pass";
        case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT: return "pipeline";
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT: return "descriptor-set-layout";
        case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT: return "sampler";
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT: return "descriptor-pool";
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT: return "descriptor-set";
        case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT: return "framebuffer";
        case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT: return "command-pool";
        case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT: return "surface-khr";
        case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT: return "swapchain-khr";
        case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT: return "debug-report";
        }
        return "unknown";
    }

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

    inline std::vector<VkExtensionProperties> availableExtensions()
    {
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> properties(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());
        return properties;
    }

    inline std::vector<VkLayerProperties> availableLayers()
    {
        uint32_t count = 0;
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        std::vector<VkLayerProperties> properties(count);
        vkEnumerateInstanceLayerProperties(&count, properties.data());
        return properties;
    }

    inline std::vector<VkPhysicalDevice> availablePhysicalDevices(VkInstance instance)
    {
        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance, &count, devices.data());
        return devices;
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
