#include "./device-holder.hpp"

#include "../helpers/queue.hpp"
#include "../helpers/swapchain.hpp"

#include <lava/magma/vr-engine.hpp>

// @note Instanciation of declared-only in vulkan.h.

VkResult vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
{
    auto Function =
        reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));
    return Function(device, pNameInfo);
}

void cmdBeginDebugUtilsLabelEXT(VkDevice device, VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabel)
{
    auto Function =
        reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT"));
    return Function(commandBuffer, pLabel);
}

void cmdEndDebugUtilsLabelEXT(VkDevice device, VkCommandBuffer commandBuffer)
{
    auto Function = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT"));
    return Function(commandBuffer);
}

using namespace lava;

namespace {
    /**
     * Checks if a device supports the extensions.
     */
    inline bool deviceExtensionsSupported(vk::PhysicalDevice physicalDevice, const std::vector<const char*>& deviceExtensions)
    {
        auto extensions = physicalDevice.enumerateDeviceExtensionProperties().value;
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
                               vk::SurfaceKHR* pSurface)
    {
        auto indices = magma::vulkan::findQueueFamilies(physicalDevice, pSurface);
        if (!indices.valid()) return false;

        auto extensionsSupported = deviceExtensionsSupported(physicalDevice, deviceExtensions);
        if (!extensionsSupported) return false;

        if (pSurface != nullptr) {
            auto swapchainSupport = magma::vulkan::swapchainSupportDetails(physicalDevice, *pSurface);
            if (!swapchainSupport.valid()) return false;
        }

        auto supportedFeatures = physicalDevice.getFeatures();
        if (!supportedFeatures.samplerAnisotropy) return false;

        return true;
    }
}

using namespace lava::magma::vulkan;
using namespace lava::chamber;

void DeviceHolder::init(vk::Instance instance, vk::SurfaceKHR* pSurface, bool debugEnabled, VrEngine& vr)
{
    m_debugEnabled = debugEnabled;

    pickPhysicalDevice(instance, pSurface);
    createLogicalDevice(pSurface, vr);
}

void DeviceHolder::debugObjectName(uint64_t object, vk::ObjectType objectType, const std::string& name) const
{
    if (!m_debugEnabled) return;

    vk::DebugUtilsObjectNameInfoEXT nameInfo;
    nameInfo.objectType = objectType;
    nameInfo.objectHandle = object;
    nameInfo.pObjectName = name.c_str();

    device().setDebugUtilsObjectNameEXT(nameInfo);
}

void DeviceHolder::debugObjectName(vk::DescriptorSet object, const std::string& name) const
{
    debugObjectName(reinterpret_cast<uint64_t&>(object), vk::ObjectType::eDescriptorSet, name);
}

void DeviceHolder::debugObjectName(vk::ImageView object, const std::string& name) const
{
    debugObjectName(reinterpret_cast<uint64_t&>(object), vk::ObjectType::eImageView, name);
}

void DeviceHolder::debugObjectName(vk::Image object, const std::string& name) const
{
    debugObjectName(reinterpret_cast<uint64_t&>(object), vk::ObjectType::eImage, name);
}

void DeviceHolder::debugObjectName(vk::Semaphore object, const std::string& name) const
{
    debugObjectName(reinterpret_cast<uint64_t&>(object), vk::ObjectType::eSemaphore, name);
}

void DeviceHolder::debugObjectName(vk::DeviceMemory object, const std::string& name) const
{
    debugObjectName(reinterpret_cast<uint64_t&>(object), vk::ObjectType::eDeviceMemory, name);
}

void DeviceHolder::debugBeginRegion(vk::CommandBuffer commandBuffer, const std::string& name) const
{
    if (!m_debugEnabled) return;

    vk::DebugUtilsLabelEXT label;
    label.pLabelName = name.c_str();

    cmdBeginDebugUtilsLabelEXT(device(), commandBuffer, &reinterpret_cast<VkDebugUtilsLabelEXT&>(label));
}

void DeviceHolder::debugEndRegion(vk::CommandBuffer commandBuffer) const
{
    if (!m_debugEnabled) return;

    cmdEndDebugUtilsLabelEXT(device(), commandBuffer);
}

void DeviceHolder::pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR* pSurface)
{
    logger.info("magma.vulkan.device-holder") << "Picking the right GPU." << std::endl;
    logger.log().tab(1);

    auto physicalDevices = instance.enumeratePhysicalDevices().value;
    logger.info("magma.vulkan.device-holder") << "Found " << physicalDevices.size() << " GPUs." << std::endl;

    if (physicalDevices.size() == 0) {
        logger.error("magma.vulkan.device-holder") << "Unable to find GPU with Vulkan support." << std::endl;
    }

    for (const auto& physicalDevice : physicalDevices) {
        if (!deviceSuitable(physicalDevice, m_extensions, pSurface)) continue;
        m_physicalDevice = physicalDevice;

        auto properties = m_physicalDevice.getProperties();
        logger.log() << "Picked GPU: " << properties.deviceName << "." << std::endl;

        auto sampleCountsFlags = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
        if (sampleCountsFlags & vk::SampleCountFlagBits::e64) { m_maxSampleCount = vk::SampleCountFlagBits::e64; }
        else if (sampleCountsFlags & vk::SampleCountFlagBits::e32) { m_maxSampleCount = vk::SampleCountFlagBits::e32; }
        else if (sampleCountsFlags & vk::SampleCountFlagBits::e16) { m_maxSampleCount = vk::SampleCountFlagBits::e16; }
        else if (sampleCountsFlags & vk::SampleCountFlagBits::e8) { m_maxSampleCount = vk::SampleCountFlagBits::e8; }
        else if (sampleCountsFlags & vk::SampleCountFlagBits::e4) { m_maxSampleCount = vk::SampleCountFlagBits::e4; }
        else if (sampleCountsFlags & vk::SampleCountFlagBits::e2) { m_maxSampleCount = vk::SampleCountFlagBits::e2; }
        logger.log() << "Max sample count: " << vk::to_string(m_maxSampleCount) << "." << std::endl;

        break;
    }

    logger.log().tab(-1);

    if (m_physicalDevice) return;

    logger.error("magma.vulkan.device-holder") << "Unable to find suitable GPU." << std::endl;
}

void DeviceHolder::createLogicalDevice(vk::SurfaceKHR* pSurface, VrEngine& vr)
{
    // Queues
    float queuePriority = 1.0f;
    m_queueFamilyIndices = findQueueFamilies(m_physicalDevice, pSurface);
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQueueFamilies = {m_queueFamilyIndices.graphics, m_queueFamilyIndices.transfer,
                                         m_queueFamilyIndices.present};

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
    deviceFeatures.fillModeNonSolid = true;

    // Extensions
    // logger.info("magma.vulkan.device-holder").tab(1) << "Available extensions:" << std::endl;
    // auto extensions = m_physicalDevice.enumerateDeviceExtensionProperties().value;
    // for (const auto& extension : extensions) {
    //     logger.log() << extension.extensionName << std::endl;
    // }
    // logger.log().tab(-1);

    auto enabledExtensions(m_extensions);

    // Checking VR extensions
    if (vr.enabled()) {
        VrRenderingNeedsInfo info;
        info.type = VrRenderingNeedsType::Vulkan;
        info.vulkan.physicalDevice = m_physicalDevice;
        auto needs = vr.renderingNeeds(info);

        for (auto i = 0u; i < needs.vulkan.deviceExtensionCount; ++i) {
            enabledExtensions.emplace_back(needs.vulkan.deviceExtensionsNames[i]);
        }
    }

    // Logging all enabled extensions
    logger.info("magma.vulkan.device-holder").tab(1) << "Enabled extensions:" << std::endl;
    for (const auto& extensionName : enabledExtensions) {
        logger.log() << extensionName << std::endl;
    }
    logger.log().tab(-1);

    // Really create
    vk::DeviceCreateInfo createInfo;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.enabledExtensionCount = enabledExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    auto result = m_physicalDevice.createDeviceUnique(createInfo);
    m_device = vulkan::checkMove(result, "device-holder", "Unable to create logical device.");

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device.get());

    m_graphicsQueue = m_device->getQueue(m_queueFamilyIndices.graphics, 0);
    m_transferQueue = m_device->getQueue(m_queueFamilyIndices.transfer, 0);
    m_presentQueue = m_device->getQueue(m_queueFamilyIndices.present, 0);
}
