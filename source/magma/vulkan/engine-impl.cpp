#include "./engine-impl.hpp"

#include <vulkan/vulkan.hpp>

using namespace lava::priv;

EngineImpl::EngineImpl()
{
    initVulkan();
}

VkResult EngineImpl::vulkanCreateInstance()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "lava-magma";
    appInfo.pEngineName = "lava-magma";
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Enable surface extensions depending on os
    // TODO Depend on OS
    std::vector<const char*> instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME};
    instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    instanceCreateInfo.enabledLayerCount = 0;

    return vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
}

void EngineImpl::initVulkan()
{
    // Vulkan instance
    auto err = vulkanCreateInstance();
    if (err) {
        // @todo Use logger vks::tools::exitFatal("Could not create Vulkan instance : \n" + vks::tools::errorString(err), "Fatal error");
    }

    // Physical device
    uint32_t gpuCount = 0;
    // Get number of available physical devices
    assert(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr) == VK_SUCCESS);
    assert(gpuCount > 0);
    // Enumerate devices
    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    err = vkEnumeratePhysicalDevices(m_instance, &gpuCount, physicalDevices.data());
    if (err) {
        // vks::tools::exitFatal("Could not enumerate physical devices : \n" + vks::tools::errorString(err), "Fatal error");
    }

    // Select physical device to be used for the Vulkan example
    // Defaults to the first device
    uint32_t selectedDevice = 0;
    auto physicalDevice = physicalDevices[selectedDevice];

    // Vulkan device creation
    // This is handled by a separate class that gets a logical device representation
    // and encapsulates functions related to a device
    m_device.bind(physicalDevice);
    VkResult res = m_device.createLogicalDevice(m_enabledFeatures, m_enabledExtensions);
    if (res != VK_SUCCESS) {
        // vks::tools::exitFatal("Could not create Vulkan device: \n" + vks::tools::errorString(res), "Fatal error");
    }

    // Get a graphics queue from the device
    auto logicalDevice = m_device.logicalDevice();
    vkGetDeviceQueue(logicalDevice, m_device.queueFamilyIndices().graphics, 0, &m_queue);

    // Find a suitable depth format
    // VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);
    // assert(validDepthFormat);

    m_swapChain.bind(*this);

    // Create synchronization objects
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queu
    vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &m_semaphores.presentComplete);
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands have been sumbitted and executed
    vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &m_semaphores.renderComplete);
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands for the text overlay have been sumbitted and executed
    // Will be inserted after the render complete semaphore if the text overlay is enabled
    vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &m_semaphores.textOverlayComplete);

    // Set up submit info structure
    // Semaphores will stay the same during application lifetime
    // Command buffer submission info is set by each example
    m_submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    m_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    m_submitInfo.pWaitDstStageMask = &m_submitPipelineStages;
    m_submitInfo.waitSemaphoreCount = 1;
    m_submitInfo.pWaitSemaphores = &m_semaphores.presentComplete;
    m_submitInfo.signalSemaphoreCount = 1;
    m_submitInfo.pSignalSemaphores = &m_semaphores.renderComplete;
}
