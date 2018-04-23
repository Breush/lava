#include "./device-holder.hpp"

#include <lava/chamber/logger.hpp>

#include "../helpers/queue.hpp"
#include "../helpers/swapchain.hpp"

// @note Instanciation of declared-only in vulkan.h.

VkResult vkDebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT* pNameInfo)
{
    auto Function =
        reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT"));
    return Function(device, pNameInfo);
}

void cmdDebugMarkerBeginEXT(VkDevice device, VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo)
{
    auto Function = reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT"));
    return Function(commandBuffer, pMarkerInfo);
}

void cmdDebugMarkerEndEXT(VkDevice device, VkCommandBuffer commandBuffer)
{
    auto Function = reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT"));
    return Function(commandBuffer);
}

using namespace lava;

namespace {
    /**
     * Checks if a device supports an extension.
     */
    inline bool deviceExtensionSupported(vk::PhysicalDevice physicalDevice, const std::string& deviceExtension)
    {
        auto extensions = physicalDevice.enumerateDeviceExtensionProperties();
        return std::find_if(extensions.begin(), extensions.end(),
                            [&deviceExtension](const vk::ExtensionProperties& extension) {
                                return extension.extensionName == deviceExtension;
                            })
               != extensions.end();
    }

    /**
     * Checks if a device supports the extensions.
     */
    inline bool deviceExtensionsSupported(vk::PhysicalDevice physicalDevice, const std::vector<const char*>& deviceExtensions)
    {
        auto extensions = physicalDevice.enumerateDeviceExtensionProperties();
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : extensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    /**
     * Checks if a device is suitable for our operations.
     */
    inline bool deviceSuitable(vk::PhysicalDevice physicalDevice, const std::vector<const char*>& deviceExtensions,
                               vk::SurfaceKHR surface)
    {
        auto indices = magma::vulkan::findQueueFamilies(physicalDevice, surface);
        if (!indices.valid()) return false;

        auto extensionsSupported = deviceExtensionsSupported(physicalDevice, deviceExtensions);
        if (!extensionsSupported) return false;

        auto swapChainSupport = magma::vulkan::swapchainSupportDetails(physicalDevice, surface);
        if (!swapChainSupport.valid()) return false;

        auto supportedFeatures = physicalDevice.getFeatures();
        if (!supportedFeatures.samplerAnisotropy) return false;

        return true;
    }
}

using namespace lava::magma::vulkan;
using namespace lava::chamber;

void DeviceHolder::init(vk::Instance instance, vk::SurfaceKHR surface)
{
    pickPhysicalDevice(instance, surface);
    createLogicalDevice(surface);
}

void DeviceHolder::debugMarkerSetObjectName(uint64_t object, vk::DebugReportObjectTypeEXT objectType,
                                            const std::string& name) const
{
    if (!m_debugMarkerExtensionEnabled) return;

    vk::DebugMarkerObjectNameInfoEXT nameInfo;
    nameInfo.objectType = objectType;
    nameInfo.object = object;
    nameInfo.pObjectName = name.c_str();
    device().debugMarkerSetObjectNameEXT(nameInfo);
}

void DeviceHolder::debugMarkerSetObjectName(vk::CommandBuffer object, const std::string& name) const
{
    debugMarkerSetObjectName(reinterpret_cast<uint64_t&>(object), vk::DebugReportObjectTypeEXT::eCommandBuffer, name);
}

void DeviceHolder::debugMarkerBeginRegion(vk::CommandBuffer commandBuffer, const std::string& name) const
{
    if (!m_debugMarkerExtensionEnabled) return;

    vk::DebugMarkerMarkerInfoEXT markerInfo;
    markerInfo.pMarkerName = name.c_str();
    cmdDebugMarkerBeginEXT(m_device.vk(), commandBuffer, &reinterpret_cast<VkDebugMarkerMarkerInfoEXT&>(markerInfo));
}

void DeviceHolder::debugMarkerEndRegion(vk::CommandBuffer commandBuffer) const
{
    if (!m_debugMarkerExtensionEnabled) return;

    cmdDebugMarkerEndEXT(m_device.vk(), commandBuffer);
}

void DeviceHolder::pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface)
{
    logger.info("magma.vulkan.device-holder") << "Picking the right GPU." << std::endl;
    logger.log().tab(1);

    auto physicalDevices = instance.enumeratePhysicalDevices();
    logger.info("magma.vulkan.device-holder") << "Found " << physicalDevices.size() << " GPUs." << std::endl;

    if (physicalDevices.size() == 0) {
        logger.error("magma.vulkan.device-holder") << "Unable to find GPU with Vulkan support." << std::endl;
    }

    for (const auto& physicalDevice : physicalDevices) {
        if (!deviceSuitable(physicalDevice, m_extensions, surface)) continue;
        m_physicalDevice = physicalDevice;

        auto properties = m_physicalDevice.getProperties();
        logger.log() << "Picked GPU: " << properties.deviceName << "." << std::endl;
        break;
    }

    logger.log().tab(-1);

    if (m_physicalDevice) return;

    logger.error("magma.vulkan.device-holder") << "Unable to find suitable GPU." << std::endl;
}

void DeviceHolder::createLogicalDevice(vk::SurfaceKHR surface)
{
    // Queues
    float queuePriority = 1.0f;
    auto indices = findQueueFamilies(m_physicalDevice, surface);
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQueueFamilies = {indices.graphics, indices.present};

    for (int queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Features
    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceFeatures.samplerAnisotropy = true;
    deviceFeatures.fragmentStoresAndAtomics = true;

    // Extensions
    auto enabledExtensions(m_extensions);

    // Check if some optional extensions can be activated
    if (deviceExtensionSupported(m_physicalDevice, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
        logger.info("magma.vulkan.device-holder")
            << "Enabling optional device extension " << VK_EXT_DEBUG_MARKER_EXTENSION_NAME << "." << std::endl;
        enabledExtensions.emplace_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        m_debugMarkerExtensionEnabled = true;
    }

    // Really create
    vk::DeviceCreateInfo createInfo;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.enabledExtensionCount = enabledExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    if (m_physicalDevice.createDevice(&createInfo, nullptr, m_device.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.device-holder") << "Unable to create logical device. " << std::endl;
    };

    m_graphicsQueue = m_device.vk().getQueue(indices.graphics, 0);
    m_presentQueue = m_device.vk().getQueue(indices.present, 0);
}
