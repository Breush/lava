#include "./engine-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <vulkan/vulkan.hpp>

#include "./proxy.hpp"
#include "./tools.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location,
                                                    int32_t code, const char* layerPrefix, const char* msg, void* userData)
{
    auto category = lava::vulkan::toString(objType);

    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        lava::logger::warning("magma.vulkan." + category) << msg << std::endl;
    }
    else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        lava::logger::error("magma.vulkan." + category) << msg << std::endl;
        exit(1);
    }

    return VK_FALSE;
}

using namespace lava::priv;

EngineImpl::EngineImpl()
{
    initVulkan();
}

void EngineImpl::initApplication(VkInstanceCreateInfo& instanceCreateInfo)
{
    m_applicationInfo = {};
    m_applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    m_applicationInfo.pApplicationName = "lava-magma";
    m_applicationInfo.pEngineName = "lava-magma";
    m_applicationInfo.apiVersion = VK_API_VERSION_1_0;

    instanceCreateInfo.pApplicationInfo = &m_applicationInfo;
}

void EngineImpl::initRequiredExtensions(VkInstanceCreateInfo& instanceCreateInfo)
{
    // Logging all available extensions
    auto availableExtensions = vulkan::availableExtensions();
    logger::info("magma.vulkan.extension") << "Available extensions:" << std::endl;
    for (const auto& extension : availableExtensions) {
        logger::info("magma.vulkan.extension") << logger::sub(1) << extension.extensionName << std::endl;
    }

    // Enable surface extensions depending on os
    // TODO Depend on OS, there should be an interface to get those somewhere
    m_instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME};
    m_instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);

    // Validation layers
    if (m_validationLayersEnabled) {
        m_instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    instanceCreateInfo.enabledExtensionCount = m_instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = m_instanceExtensions.data();

    // Logging all enabled extensions
    logger::info("magma.vulkan.extension") << "Enabled extensions:" << std::endl;
    for (const auto& extensionName : m_instanceExtensions) {
        logger::info("magma.vulkan.extension") << logger::sub(1) << extensionName << std::endl;
    }
}

void EngineImpl::initValidationLayers(VkInstanceCreateInfo& instanceCreateInfo)
{
    instanceCreateInfo.enabledLayerCount = 0;

    if (!m_validationLayersEnabled) return;

    if (!vulkan::validationLayersSupported(m_validationLayers)) {
        logger::warning("magma.vulkan.layer") << "Validation layers enabled, but are not available." << std::endl;
        m_validationLayersEnabled = false;
        return;
    }

    instanceCreateInfo.enabledLayerCount = m_validationLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();

    logger::info("magma.vulkan.layer") << "Validation layers enabled." << std::endl;
}

void EngineImpl::createInstance()
{
    // Optional data
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "lava-magma";
    appInfo.pEngineName = "lava-magma";
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    // Initialization of extensions and layers
    initValidationLayers(instanceCreateInfo);
    initRequiredExtensions(instanceCreateInfo);

    // Really create the instance
    auto err = vkCreateInstance(&instanceCreateInfo, nullptr, m_instance.replace());
    if (!err) return;

    // @todo Have a way to have this exit(1) included! And debug trace?
    logger::error("magma.vulkan") << "Could not create Vulkan instance. " << vulkan::toString(err) << std::endl;
    exit(1);
}

void EngineImpl::setupDebug()
{
    if (!m_validationLayersEnabled) return;

    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugCallback;

    vulkan::CreateDebugReportCallbackEXT(m_instance, &createInfo, nullptr, m_debugReportCallback.replace());
}

void EngineImpl::pickPhysicalDevice()
{
    auto devices = vulkan::availablePhysicalDevices(m_instance);

    if (devices.size() == 0) {
        logger::error("magma.vulkan.physical-device") << "Unable to find GPU with Vulkan support." << std::endl;
        exit(1);
    }

    for (const auto& device : devices) {
        if (!vulkan::deviceSuitable(device)) continue;
        m_physicalDevice = device;
        break;
    }

    if (m_physicalDevice == VK_NULL_HANDLE) {
        logger::error("magma.vulkan.physical-device") << "Unable to find suitable GPU." << std::endl;
        exit(1);
    }
}

void EngineImpl::initVulkan()
{
    createInstance();
    setupDebug();
    pickPhysicalDevice();

    /*// Vulkan device creation
    // This is handled by a separate class that gets a logical device representation
    // and encapsulates functions related to a device
    m_device.bind(physicalDevice);
    VkResult res = m_device.createLogicalDevice(m_enabledFeatures, m_enabledExtensions);
    if (res != VK_SUCCESS) {
        logger::error("magma.vulkan") << "Could not create Vulkan device. " << vulkan::toString(err) << std::endl;
        exit(1);
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
    m_submitInfo.pSignalSemaphores = &m_semaphores.renderComplete;*/
}
