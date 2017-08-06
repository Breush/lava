#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace lava::magma::vulkan {

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

    inline uint32_t findMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
    {
        auto memProperties = physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        return -1;
    }
}
