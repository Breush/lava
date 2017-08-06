#include "./device-holder.hpp"

#include <lava/chamber/logger.hpp>

#include "./queue.hpp"
#include "./swapchain-support-details.hpp"
#include "./tools.hpp"

using namespace lava;

namespace {
    /**
     * Checks if a device supports the extensions.
     */
    inline bool deviceExtensionsSupported(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions)
    {
        auto extensions = magma::vulkan::availableExtensions(device);
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
        auto indices = magma::vulkan::findQueueFamilies(device, surface);
        if (!indices.valid()) return false;

        auto extensionsSupported = deviceExtensionsSupported(device, deviceExtensions);
        if (!extensionsSupported) return false;

        auto swapChainSupport = magma::vulkan::swapchainSupportDetails(device, surface);
        if (!swapChainSupport.valid()) return false;

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
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

void DeviceHolder::pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface)
{
    auto physicalDevices = availablePhysicalDevices(instance);

    if (physicalDevices.size() == 0) {
        logger.error("magma.vulkan.physical-device") << "Unable to find GPU with Vulkan support." << std::endl;
    }

    for (const auto& physicalDevice : physicalDevices) {
        if (!deviceSuitable(physicalDevice, m_extensions, surface)) continue;
        m_physicalDevice = physicalDevice;
        break;
    }

    if (m_physicalDevice) return;

    logger.error("magma.vulkan.physical-device") << "Unable to find suitable GPU." << std::endl;
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

    // Really create
    vk::DeviceCreateInfo createInfo;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.enabledExtensionCount = m_extensions.size();
    createInfo.ppEnabledExtensionNames = m_extensions.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    if (m_physicalDevice.createDevice(&createInfo, nullptr, m_device.replace()) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.device-holder") << "Unable to create logical device. " << std::endl;
    };

    m_graphicsQueue = m_device.vk().getQueue(indices.graphics, 0);
    m_presentQueue = m_device.vk().getQueue(indices.present, 0);
}
