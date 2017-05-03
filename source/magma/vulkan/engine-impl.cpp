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
    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    auto indices = vulkan::findQueueFamilies(m_physicalDevice, m_surface);
    std::vector<uint32_t> queueFamilyIndices = {(uint32_t)indices.graphics, (uint32_t)indices.present};

    auto sameFamily = (indices.graphics == indices.present);
    createInfo.imageSharingMode = sameFamily ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = sameFamily ? 0 : queueFamilyIndices.size();
    createInfo.pQueueFamilyIndices = sameFamily ? nullptr : queueFamilyIndices.data();

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, m_swapChain.replace()) != VK_SUCCESS) {
        logger::error("magma.vulkan.swap-chain") << "Failed to create swap chain." << std::endl;
        exit(1);
    }

    // Retrieving image handles (we need to request the real image count as the implementation can require more)
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

    // Saving some values
    m_swapChainExtent = extent;
    m_swapChainImageFormat = surfaceFormat.format;
}

void EngineImpl::createImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size(), vulkan::Capsule<VkImageView>{m_device, vkDestroyImageView});

    for (uint32_t i = 0; i < m_swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device, &createInfo, nullptr, m_swapChainImageViews[i].replace()) != VK_SUCCESS) {
            logger::error("magma.vulkan.image-view") << "Failed to create image views." << std::endl;
        }
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
    createImageViews();
}
