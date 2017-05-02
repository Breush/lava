#include "./engine-impl.hpp"

#include <lava/chamber/logger.hpp>
#include <set>
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

EngineImpl::EngineImpl(lava::Window& window)
    : m_windowHandle(window.getSystemHandle())
    , m_windowExtent({window.videoMode().width, window.videoMode().height})
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

void EngineImpl::createSurface()
{
    // @todo This is platform-specific!
    VkXcbSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.connection = m_windowHandle.connection;
    createInfo.window = m_windowHandle.window;

    auto CreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkCreateXcbSurfaceKHR");
    auto err = CreateXcbSurfaceKHR(m_instance, &createInfo, nullptr, m_surface.replace());
    if (!err) return;

    logger::error("magma.vulkan.surface") << "Unable to create surface for platform." << std::endl;
    exit(1);
}

void EngineImpl::pickPhysicalDevice()
{
    auto devices = vulkan::availablePhysicalDevices(m_instance);

    if (devices.size() == 0) {
        logger::error("magma.vulkan.physical-device") << "Unable to find GPU with Vulkan support." << std::endl;
        exit(1);
    }

    for (const auto& device : devices) {
        if (!vulkan::deviceSuitable(device, m_deviceExtensions, m_surface)) continue;
        m_physicalDevice = device;
        break;
    }

    if (m_physicalDevice != VK_NULL_HANDLE) return;

    logger::error("magma.vulkan.physical-device") << "Unable to find suitable GPU." << std::endl;
    exit(1);
}

void EngineImpl::createLogicalDevice()
{
    // Device
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    // Queues
    float queuePriority = 1.0f;
    auto indices = vulkan::findQueueFamilies(m_physicalDevice, m_surface);
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
    createInfo.enabledExtensionCount = m_deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

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

void EngineImpl::createSwapChain()
{
    auto details = vulkan::swapChainSupportDetails(m_physicalDevice, m_surface);

    auto surfaceFormat = vulkan::swapChainSurfaceFormat(details.formats);
    auto presentMode = vulkan::swapChainPresentMode(details.presentModes);
    auto extent = vulkan::swapChainExtent(details.capabilities, m_windowExtent);

    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
        imageCount = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto indices = vulkan::findQueueFamilies(m_physicalDevice, m_surface);
    std::vector<uint32_t> queueFamilyIndices = {(uint32_t)indices.graphics, (uint32_t)indices.present};

    if (indices.graphics != indices.present) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
}

void EngineImpl::initVulkan()
{
    createInstance();
    setupDebug();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
}
