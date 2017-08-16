#include "./instance-holder.hpp"

#include <lava/chamber/logger.hpp>
#include <lava/chamber/string-tools.hpp>

// @note Instanciation of declared-only in vulkan.h.

VkResult vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
    auto CreateDebugReportCallback =
        reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
    return CreateDebugReportCallback(instance, pCreateInfo, pAllocator, pCallback);
}

void vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                     const VkAllocationCallbacks* pAllocator)
{
    auto DestroyDebugReportCallback =
        reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
    return DestroyDebugReportCallback(instance, callback, pAllocator);
}

using namespace lava;

namespace {
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t,
                                                 size_t, int32_t, const char*, const char* msg, void*)
    {
        auto vk_flags = reinterpret_cast<vk::DebugReportFlagsEXT&>(flags);
        auto vk_objType = reinterpret_cast<vk::DebugReportObjectTypeEXT&>(objType);
        auto category = chamber::camelToSnakeCase(vk::to_string(vk_objType));

        if (vk_flags & vk::DebugReportFlagBitsEXT::eWarning) {
            chamber::logger.warning("magma.vulkan." + category) << msg << std::endl;
        }
        else if (vk_flags & vk::DebugReportFlagBitsEXT::eError) {
            chamber::logger.error("magma.vulkan." + category) << msg << std::endl;
        }

        return VK_FALSE;
    }

    bool layersSupported(const std::vector<const char*>& layers)
    {
        auto availableLayers = vk::enumerateInstanceLayerProperties();

        for (auto layerName : layers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName)) continue;
                layerFound = true;
                break;
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }
}

using namespace lava::magma::vulkan;
using namespace lava::chamber;

void InstanceHolder::init(bool debugEnabled)
{
    m_debugEnabled = debugEnabled;

    createInstance();
    setupDebug();
}

void InstanceHolder::createInstance()
{
    vk::InstanceCreateInfo instanceCreateInfo;
    initApplication(instanceCreateInfo);
    initValidationLayers(instanceCreateInfo);
    initRequiredExtensions(instanceCreateInfo);

    // Really create the instance
    vk::Result result;
    if ((result = vk::createInstance(&instanceCreateInfo, nullptr, m_instance.replace())) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.instance-holder") << "Could not create instance. " << vk::to_string(result) << std::endl;
    }
}

void InstanceHolder::setupDebug()
{
    if (!m_debugEnabled) return;

    vk::DebugReportCallbackCreateInfoEXT createInfo;
    createInfo.flags = vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning;
    createInfo.pfnCallback = reinterpret_cast<PFN_vkDebugReportCallbackEXT>(debugCallback);

    m_instance.vk().createDebugReportCallbackEXT(&createInfo, nullptr, m_debugReportCallback.replace());
}

void InstanceHolder::initApplication(vk::InstanceCreateInfo& instanceCreateInfo)
{
    m_applicationInfo.pApplicationName = "lava-magma";
    m_applicationInfo.pEngineName = "lava-magma";
    m_applicationInfo.apiVersion = VK_API_VERSION_1_0;

    instanceCreateInfo.pApplicationInfo = &m_applicationInfo;
}

void InstanceHolder::initRequiredExtensions(vk::InstanceCreateInfo& instanceCreateInfo)
{
    // Logging all available extensions
    auto extensions = vk::enumerateInstanceExtensionProperties();
    logger.info("magma.vulkan.extension") << "Available extensions:" << std::endl;
    logger.log().tab(1);
    for (const auto& extension : extensions) {
        logger.log() << extension.extensionName << std::endl;
    }
    logger.log().tab(-1);

    m_extensions = {VK_KHR_SURFACE_EXTENSION_NAME};
#if defined(VK_USE_PLATFORM_XCB_KHR)
    m_extensions.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    m_extensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

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

void InstanceHolder::initValidationLayers(vk::InstanceCreateInfo& instanceCreateInfo)
{
    instanceCreateInfo.enabledLayerCount = 0;

    if (!m_debugEnabled) return;

    if (!layersSupported(m_validationLayers)) {
        logger.warning("magma.vulkan.layer") << "Validation layers enabled, but are not available." << std::endl;
        m_debugEnabled = false;
        return;
    }

    instanceCreateInfo.enabledLayerCount = m_validationLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();

    logger.info("magma.vulkan.layer") << "Validation layers enabled." << std::endl;
}
