#include "./instance-holder.hpp"

#include <lava/magma/vr-engine.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

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
    uint32_t debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                           vk::DebugUtilsMessageTypeFlagBitsEXT messageType,
                           const vk::DebugUtilsMessengerCallbackDataEXT* callbackData, void*)
    {
        auto type = "magma.vulkan.debug." + chamber::camelToSnakeCase(vk::to_string(messageType));
        auto message = "[" + std::string(callbackData->pMessageIdName) + "] " + callbackData->pMessage;

        std::string context;
        for (auto i = 0u; i < callbackData->objectCount; ++i) {
            const auto& object = callbackData->pObjects[i];
            context += "| " + vk::to_string(object.objectType) + " ";
            context += (object.pObjectName) ? object.pObjectName : "<Unknown>";
            context += '\n';
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

        return 0;
    }

    bool layersSupported(const std::vector<const char*>& layers)
    {
        auto availableLayers = vk::enumerateInstanceLayerProperties().value;

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

void InstanceHolder::init(bool debugEnabled, VrEngine& vr)
{
    m_debugEnabled = debugEnabled;

    auto vkGetInstanceProcAddr = m_dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    createInstance(vr);
    setupDebug(vr);
}

void InstanceHolder::createInstance(VrEngine& vr)
{
    vk::InstanceCreateInfo instanceCreateInfo;
    initApplication(instanceCreateInfo);
    initValidationLayers(instanceCreateInfo, vr);
    initRequiredExtensions(instanceCreateInfo, vr);

    // Really create the instance
    auto result = vk::createInstanceUnique(instanceCreateInfo);
    m_instance = vulkan::checkMove(result, "instance-holder", "Unable to create instance.");

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance.get());
}

void InstanceHolder::setupDebug(VrEngine& vr)
{
    if (!m_debugEnabled) return;

    vk::DebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

    // @note We can't show Performance issues when using VR, because OpenVR/SteamVR makes it emit a warning at each loop.
    if (!vr.enabled()) {
        createInfo.messageType |= vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    }

    createInfo.messageSeverity =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    createInfo.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(debugCallback);

    auto result = m_instance->createDebugUtilsMessengerEXTUnique(createInfo);
    m_debugUtilsMessenger = vulkan::checkMove(result, "instance-holder", "Unable to set debug up.");
}

void InstanceHolder::initApplication(vk::InstanceCreateInfo& instanceCreateInfo)
{
    m_applicationInfo.pApplicationName = "lava-magma";
    m_applicationInfo.pEngineName = "lava-magma";
    m_applicationInfo.apiVersion = VK_API_VERSION_1_2;

    instanceCreateInfo.pApplicationInfo = &m_applicationInfo;
}

void InstanceHolder::initRequiredExtensions(vk::InstanceCreateInfo& instanceCreateInfo, VrEngine& vr)
{
    // Logging all available extensions
    bool debugUtilsExtensionAvailable = false;
    auto extensions = vk::enumerateInstanceExtensionProperties().value;
    logger.info("magma.vulkan.instance-holder").tab(1) << "Available extensions:" << std::endl;
    for (const auto& extension : extensions) {
        debugUtilsExtensionAvailable = debugUtilsExtensionAvailable || (std::string(extension.extensionName) == VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        logger.log() << extension.extensionName << std::endl;
    }
    logger.log().tab(-1);

    // @fixme The surface extension might not always be needed...
    m_extensions = {VK_KHR_SURFACE_EXTENSION_NAME};
#if defined(VK_USE_PLATFORM_XCB_KHR)
    m_extensions.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    m_extensions.emplace_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    m_extensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

    if (vr.enabled()) {
        VrRenderingNeedsInfo info;
        info.type = VrRenderingNeedsType::Vulkan;
        info.vulkan.physicalDevice = nullptr;
        auto needs = vr.renderingNeeds(info);

        for (auto i = 0u; i < needs.vulkan.instanceExtensionCount; ++i) {
            m_extensions.emplace_back(needs.vulkan.instanceExtensionsNames[i]);
        }
    }

    // Validation layers
    if (m_debugEnabled) {
        if (debugUtilsExtensionAvailable) {
            m_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else {
            m_debugEnabled = false;
            logger.warning("magma.vulkan.instance-holder")
                << "Disabling debug because " VK_EXT_DEBUG_UTILS_EXTENSION_NAME << " extension is not available." << std::endl;
        }
    }

    instanceCreateInfo.enabledExtensionCount = m_extensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = m_extensions.data();

    // Logging all enabled extensions
    logger.info("magma.vulkan.instance-holder").tab(1) << "Enabled extensions:" << std::endl;
    for (const auto& extensionName : m_extensions) {
        logger.log() << extensionName << std::endl;
    }
    logger.log().tab(-1);
}

void InstanceHolder::initValidationLayers(vk::InstanceCreateInfo& instanceCreateInfo, VrEngine& vr)
{
    instanceCreateInfo.enabledLayerCount = 0;

    if (!m_debugEnabled) return;

#if defined(PROFILE_ENABLED)
    logger.warning("magma.vulkan.instance-holder") << "Validation layers enabled, but profiling is enabled too. " << std::endl;
    logger.log() << "We don't enable validation layers as they add a way too big overhead." << std::endl;
    m_debugEnabled = false;
    return;
#endif

    if (vr.enabled()) {
        logger.warning("magma.vulkan.instance-holder") << "Validation layers enabled, but VR is enabled too. " << std::endl;
        logger.log() << "We don't enable validation layers as they add a way too big overhead." << std::endl;
        m_debugEnabled = false;
        return;
    }

    if (!layersSupported(m_validationLayers)) {
        logger.warning("magma.vulkan.instance-holder") << "Validation layers enabled, but are not available." << std::endl;
        m_debugEnabled = false;
        return;
    }

    instanceCreateInfo.enabledLayerCount = m_validationLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();

    logger.info("magma.vulkan.instance-holder") << "Validation layers enabled." << std::endl;
}
