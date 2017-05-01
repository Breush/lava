#pragma once

#include <lava/crater/Window.hpp>

#include "./capsule.hpp"
#include "./device.hpp"
#include "./proxy.hpp"

namespace lava::priv {
    /**
     * Vulkan-based implementation of the lava::Engine.
     */
    class EngineImpl {
    public:
        EngineImpl(lava::Window& window);

        inline vulkan::Capsule<VkInstance>& instance() { return m_instance; }

    protected:
        void initVulkan();
        void setupDebug();
        void createSurface();

        void createInstance();
        void initApplication(VkInstanceCreateInfo& instanceCreateInfo);
        void initValidationLayers(VkInstanceCreateInfo& instanceCreateInfo);
        void initRequiredExtensions(VkInstanceCreateInfo& instanceCreateInfo);

        void pickPhysicalDevice();
        void createLogicalDevice();

    private:
        lava::WindowHandle m_windowHandle;

        // Instance-related
        vulkan::Capsule<VkInstance> m_instance{vkDestroyInstance};
        VkApplicationInfo m_applicationInfo;
        std::vector<const char*> m_instanceExtensions;

        // Validation layers
        bool m_validationLayersEnabled = true;
        const std::vector<const char*> m_validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
        vulkan::Capsule<VkDebugReportCallbackEXT> m_debugReportCallback{m_instance, vulkan::DestroyDebugReportCallbackEXT};

        // Devices
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        vulkan::Capsule<VkDevice> m_device{vkDestroyDevice};
        VkQueue m_graphicsQueue;

        // Surfaces
        vulkan::Capsule<VkSurfaceKHR> m_surface{m_instance, vkDestroySurfaceKHR};
        VkQueue m_presentQueue;
    };
}
