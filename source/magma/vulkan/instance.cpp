#include "./instance.hpp"

#include <lava/chamber/logger.hpp>

#include "./tools.hpp"

namespace {
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t,
                                                        size_t, int32_t, const char*, const char* msg, void*)
    {
        auto category = lava::vulkan::toString(objType);

        if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
            lava::logger.warning("magma.vulkan." + category) << msg << std::endl;
        }
        else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
            lava::logger.error("magma.vulkan." + category) << msg << std::endl;
            exit(1);
        }

        return VK_FALSE;
    }
}

using namespace lava::vulkan;

void Instance::init(bool debugEnabled)
{
    m_debugEnabled = debugEnabled;

    createInstance();
    setupDebug();
}

void Instance::createInstance()
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
    logger.error("magma.vulkan") << "Could not create Vulkan instance. " << toString(err) << std::endl;
    exit(1);
}

void Instance::setupDebug()
{
    if (!m_debugEnabled) return;

    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugCallback;

    CreateDebugReportCallbackEXT(m_instance, &createInfo, nullptr, m_debugReportCallback.replace());
}

void Instance::initApplication(VkInstanceCreateInfo& instanceCreateInfo)
{
    m_applicationInfo = {};
    m_applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    m_applicationInfo.pApplicationName = "lava-magma";
    m_applicationInfo.pEngineName = "lava-magma";
    m_applicationInfo.apiVersion = VK_API_VERSION_1_0;

    instanceCreateInfo.pApplicationInfo = &m_applicationInfo;
}

void Instance::initRequiredExtensions(VkInstanceCreateInfo& instanceCreateInfo)
{
    // Logging all available extensions
    auto extensions = availableExtensions();
    logger.info("magma.vulkan.extension") << "Available extensions:" << std::endl;
    logger.log().tab(1);
    for (const auto& extension : extensions) {
        logger.log() << extension.extensionName << std::endl;
    }
    logger.log().tab(-1);

    // Enable surface extensions depending on os
    // TODO Depend on OS, there should be an interface to get those somewhere
    m_extensions = {VK_KHR_SURFACE_EXTENSION_NAME};
    m_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);

    // Validation layers
    if (m_debugEnabled) {
        m_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    instanceCreateInfo.enabledExtensionCount = m_extensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = m_extensions.data();

    // Logging all enabled extensions
    logger.log() << "Enabled extensions:" << std::endl;
    logger.log().tab(1);
    for (const auto& extensionName : m_extensions) {
        logger.log() << extensionName << std::endl;
    }
    logger.log().tab(-1);
}

void Instance::initValidationLayers(VkInstanceCreateInfo& instanceCreateInfo)
{
    instanceCreateInfo.enabledLayerCount = 0;

    if (!m_debugEnabled) return;

    if (!validationLayersSupported(m_validationLayers)) {
        logger.warning("magma.vulkan.layer") << "Validation layers enabled, but are not available." << std::endl;
        m_debugEnabled = false;
        return;
    }

    instanceCreateInfo.enabledLayerCount = m_validationLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();

    logger.info("magma.vulkan.layer") << "Validation layers enabled." << std::endl;
}
