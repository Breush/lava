#include "./device.hpp"

using namespace lava;

Device::Device()
{
}

Device::~Device()
{
    if (m_logicalDevice) vkDestroyDevice(m_logicalDevice, nullptr);
}

void Device::bind(VkPhysicalDevice physicalDevice)
{
    m_physicalDevice = physicalDevice;

    if (!physicalDevice) return;

    vkGetPhysicalDeviceProperties(physicalDevice, &m_properties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &m_features);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &m_memoryProperties);

    /*// Queue family properties, used for setting up requested queues upon device creation
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    assert(queueFamilyCount > 0);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    // Get list of supported extensions
    uint32_t extCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
    if (extCount > 0) {
        std::vector<VkExtensionProperties> extensions(extCount);
        if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS) {
            for (auto ext : extensions) {
                supportedExtensions.push_back(ext.extensionName);
            }
        }
    }*/
}

VkResult Device::createLogicalDevice(VkPhysicalDeviceFeatures& enabledFeatures, std::vector<const char*>& enabledExtensions)
{
    VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;

    // Desired queues need to be requested upon logical device creation
    // Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
    // requests different queue types

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

    // Get queue family indices for the requested queue family types
    // Note that the indices may overlap depending on the implementation

    const float defaultQueuePriority(0.0f);

    // Graphics queue
    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
        m_queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo queueInfo;
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = m_queueFamilyIndices.graphics;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
    }
    else {
        m_queueFamilyIndices.graphics = VK_NULL_HANDLE;
    }

    // Dedicated compute queue
    if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
        m_queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
        if (m_queueFamilyIndices.compute != m_queueFamilyIndices.graphics) {
            // If compute family index differs, we need an additional queue create info for the compute queue
            VkDeviceQueueCreateInfo queueInfo;
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = m_queueFamilyIndices.compute;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
    }
    else {
        // Else we use the same queue
        m_queueFamilyIndices.compute = m_queueFamilyIndices.graphics;
    }

    // Dedicated transfer queue
    if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
        m_queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
        if ((m_queueFamilyIndices.transfer != m_queueFamilyIndices.graphics) && (m_queueFamilyIndices.transfer != m_queueFamilyIndices.compute)) {
            // If compute family index differs, we need an additional queue create info for the compute queue
            VkDeviceQueueCreateInfo queueInfo;
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = m_queueFamilyIndices.transfer;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
    }
    else {
        // Else we use the same queue
        m_queueFamilyIndices.transfer = m_queueFamilyIndices.graphics;
    }

    // Create the logical device representation
    std::vector<const char*> deviceExtensions(enabledExtensions);
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

    // Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
    if (extensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
        deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        // enableDebugMarkers = true;
    }

    if (deviceExtensions.size() > 0) {
        deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    }

    VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice);

    if (result == VK_SUCCESS) {
        // Create a default command pool for graphics command buffers
        // @todo commandPool = createCommandPool(m_queueFamilyIndices.graphics);
    }

    return result;
}

uint32_t Device::getQueueFamilyIndex(VkQueueFlagBits queueFlags)
{
    auto length = static_cast<uint32_t>(m_queueFamilyProperties.size());

    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if (queueFlags & VK_QUEUE_COMPUTE_BIT) {
        for (uint32_t i = 0; i < length; i++) {
            auto flags = m_queueFamilyProperties[i].queueFlags;
            if ((flags & queueFlags) && !(flags & VK_QUEUE_GRAPHICS_BIT)) {
                return i;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if (queueFlags & VK_QUEUE_TRANSFER_BIT) {
        for (uint32_t i = 0; i < length; i++) {
            auto flags = m_queueFamilyProperties[i].queueFlags;
            if ((flags & queueFlags) && !(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_COMPUTE_BIT)) {
                return i;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for (uint32_t i = 0; i < length; i++) {
        if (m_queueFamilyProperties[i].queueFlags & queueFlags) {
            return i;
        }
    }

    // @todo
    // throw std::runtime_error("Could not find a matching queue family index");
    return 0;
}

bool Device::extensionSupported(const std::string& extension)
{
    return (std::find(m_supportedExtensions.begin(), m_supportedExtensions.end(), extension) != m_supportedExtensions.end());
}
