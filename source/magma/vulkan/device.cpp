#include "./device.hpp"

#include <lava/chamber/logger.hpp>

#include "./queue.hpp"
#include "./swap-chain.hpp"
#include "./tools.hpp"

namespace {
    /**
     * Checks if a device supports the extensions.
     */
    inline bool deviceExtensionsSupported(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions)
    {
        auto extensions = lava::vulkan::availableExtensions(device);
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : extensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    /**
     * Checks if a device is suitable for our operations.
     */
    inline bool deviceSuitable(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions, VkSurfaceKHR surface)
    {
        auto indices = lava::vulkan::findQueueFamilies(device, surface);
        if (!indices.valid()) return false;

        auto extensionsSupported = deviceExtensionsSupported(device, deviceExtensions);
        if (!extensionsSupported) return false;

        auto swapChainSupport = lava::vulkan::swapChainSupportDetails(device, surface);
        if (!swapChainSupport.valid()) false;

        return true;
    }
}

using namespace lava::vulkan;

void Device::init(VkInstance instance, VkSurfaceKHR surface)
{
    pickPhysicalDevice(instance, surface);
    createLogicalDevice(surface);
}

void Device::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    auto devices = availablePhysicalDevices(instance);

    if (devices.size() == 0) {
        logger::error("magma.vulkan.physical-device") << "Unable to find GPU with Vulkan support." << std::endl;
        exit(1);
    }

    for (const auto& device : devices) {
        if (!deviceSuitable(device, m_extensions, surface)) continue;
        m_physicalDevice = device;
        break;
    }

    if (m_physicalDevice != VK_NULL_HANDLE) return;

    logger::error("magma.vulkan.physical-device") << "Unable to find suitable GPU." << std::endl;
    exit(1);
}

void Device::createLogicalDevice(VkSurfaceKHR surface)
{
    // Device
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    // Queues
    float queuePriority = 1.0f;
    auto indices = findQueueFamilies(m_physicalDevice, surface);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQueueFamilies = {indices.graphics, indices.present};

    for (int queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();

    // Extensions
    createInfo.enabledExtensionCount = m_extensions.size();
    createInfo.ppEnabledExtensionNames = m_extensions.data();

    // Features
    VkPhysicalDeviceFeatures deviceFeatures = {};

    createInfo.pEnabledFeatures = &deviceFeatures;

    // Really create
    auto err = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, m_device.replace());
    if (err) {
        logger::error("magma.vulkan.device") << "Unable to create logical device. " << vulkan::toString(err) << std::endl;
        exit(1);
    };

    vkGetDeviceQueue(m_device, indices.graphics, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.present, 0, &m_presentQueue);
}
