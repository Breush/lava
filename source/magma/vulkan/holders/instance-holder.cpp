#include "./instance-holder.hpp"

#include <lava/chamber/logger.hpp>
#include <lava/chamber/string-tools.hpp>

// @note Instanciation of declared-only in vulkan.h.

VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
{
    auto CreateDebugUtilsMessenger =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    return CreateDebugUtilsMessenger(instance, pCreateInfo, pAllocator, pCallback);
}

void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback,
                                     const VkAllocationCallbacks* pAllocator)
{
    auto DestroyDebugUtilsMessenger =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    return DestroyDebugUtilsMessenger(instance, callback, pAllocator);
}

using namespace lava;

namespace {
    bool debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagBitsEXT messageType,
                       const vk::DebugUtilsMessengerCallbackDataEXT* callbackData, void*)
    {
        auto type = "magma.vulkan.debug." + chamber::camelToSnakeCase(vk::to_string(messageType));
        auto message = "[" + std::string(callbackData->pMessageIdName) + "] " + callbackData->pMessage;

        std::string context;
        for (auto i = 0u; i < callbackData->objectCount; ++i) {
            const auto& object = callbackData->pObjects[i];
            context += "| " + vk::to_string(object.objectType) + " ";
            context += (object.pObjectName) ? object.pObjectName : "<Unknown>";
            context += "\n";
        }

        if (messageSeverity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
            if (!context.empty()) chamber::logger.warning(type) << context;
            chamber::logger.error(type) << message << std::endl;
        }
        else if (messageSeverity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
            if (!context.empty()) chamber::logger.warning(type) << context;
            chamber::logger.warning(type) << message << std::endl;
        }
        else {
            chamber::logger.info(type) << message << std::endl;
        }

        return false;
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

    vk::DebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                             | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    createInfo.messageSeverity =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    createInfo.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(debugCallback);

    m_instance.vk().createDebugUtilsMessengerEXT(&createInfo, nullptr, m_debugUtilsMessenger.replace());
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
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    m_extensions.emplace_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    m_extensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

    // Validation layers
    if (m_debugEnabled) {
        m_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
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
        logger.warning("magma.vulkan.instance-holder") << "Validation layers enabled, but are not available." << std::endl;
        m_debugEnabled = false;
        return;
    }

    instanceCreateInfo.enabledLayerCount = m_validationLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();

    logger.info("magma.vulkan.instance-holder") << "Validation layers enabled." << std::endl;
}
