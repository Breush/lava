#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace lava::vulkan {

    inline std::string toString(vk::DebugReportObjectTypeEXT objType)
    {
        switch (objType) {
        case vk::DebugReportObjectTypeEXT::eInstance: return "instance";
        case vk::DebugReportObjectTypeEXT::ePhysicalDevice: return "physical-device";
        case vk::DebugReportObjectTypeEXT::eDevice: return "device";
        case vk::DebugReportObjectTypeEXT::eQueue: return "queue";
        case vk::DebugReportObjectTypeEXT::eSemaphore: return "semaphore";
        case vk::DebugReportObjectTypeEXT::eCommandBuffer: return "command-buffer";
        case vk::DebugReportObjectTypeEXT::eFence: return "fence";
        case vk::DebugReportObjectTypeEXT::eDeviceMemory: return "device-memory";
        case vk::DebugReportObjectTypeEXT::eBuffer: return "buffer";
        case vk::DebugReportObjectTypeEXT::eImage: return "image";
        case vk::DebugReportObjectTypeEXT::eEvent: return "event";
        case vk::DebugReportObjectTypeEXT::eQueryPool: return "query-pool";
        case vk::DebugReportObjectTypeEXT::eBufferView: return "buffer-view";
        case vk::DebugReportObjectTypeEXT::eImageView: return "image-view";
        case vk::DebugReportObjectTypeEXT::eShaderModule: return "shader-module";
        case vk::DebugReportObjectTypeEXT::ePipelineCache: return "pipeline-cache";
        case vk::DebugReportObjectTypeEXT::ePipelineLayout: return "pipeline-layout";
        case vk::DebugReportObjectTypeEXT::eRenderPass: return "render-pass";
        case vk::DebugReportObjectTypeEXT::ePipeline: return "pipeline";
        case vk::DebugReportObjectTypeEXT::eDescriptorSetLayout: return "descriptor-set-layout";
        case vk::DebugReportObjectTypeEXT::eSampler: return "sampler";
        case vk::DebugReportObjectTypeEXT::eDescriptorPool: return "descriptor-pool";
        case vk::DebugReportObjectTypeEXT::eDescriptorSet: return "descriptor-set";
        case vk::DebugReportObjectTypeEXT::eFramebuffer: return "framebuffer";
        case vk::DebugReportObjectTypeEXT::eCommandPool: return "command-pool";
        case vk::DebugReportObjectTypeEXT::eSurfaceKhr: return "surface-khr";
        case vk::DebugReportObjectTypeEXT::eSwapchainKhr: return "swapchain-khr";
        case vk::DebugReportObjectTypeEXT::eDebugReportCallbackExt: return "debug-report-callback-ext";
        case vk::DebugReportObjectTypeEXT::eDisplayKhr: return "display-khr";
        case vk::DebugReportObjectTypeEXT::eDisplayModeKhr: return "display-mode-khr";
        case vk::DebugReportObjectTypeEXT::eObjectTableNvx: return "object-table-nvx";
        case vk::DebugReportObjectTypeEXT::eIndirectCommandsLayoutNvx: return "indirect-commands-layout-nvx";
        case vk::DebugReportObjectTypeEXT::eDescriptorUpdateTemplateKHR: return "descriptor-update-template-khr";
        default: break;
        }

        return "unknown";
    }

    /**
     * Convert a VkResult to a string.
     */
    inline std::string toString(VkResult errorCode)
    {
        switch (errorCode) {
#define STR(r)                                                                                                                   \
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
        std::vector<VkExtensionProperties> extensions(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
        return extensions;
    }

    inline std::vector<VkExtensionProperties> availableExtensions(VkPhysicalDevice device)
    {
        uint32_t count = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> extensions(count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data());
        return extensions;
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

    inline std::vector<VkQueueFamilyProperties> availableQueueFamilies(VkPhysicalDevice device)
    {
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());
        return queueFamilies;
    }

    inline bool validationLayersSupported(const std::vector<const char*>& validationLayers)
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

    inline uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        return -1;
    }
}
