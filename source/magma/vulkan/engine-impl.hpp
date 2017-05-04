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

        void createInstance();
        void initApplication(VkInstanceCreateInfo& instanceCreateInfo);
        void initValidationLayers(VkInstanceCreateInfo& instanceCreateInfo);
        void initRequiredExtensions(VkInstanceCreateInfo& instanceCreateInfo);

        void pickPhysicalDevice();
        void createLogicalDevice();

        void createSurface();
        void createSwapChain();
        void createImageViews();

        void createGraphicsPipeline();

    private:
        lava::WindowHandle m_windowHandle;
        VkExtent2D m_windowExtent;

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
        const std::vector<const char*> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        VkQueue m_graphicsQueue;

        // Surfaces (and swap chain)
        vulkan::Capsule<VkSurfaceKHR> m_surface{m_instance, vkDestroySurfaceKHR};
        VkQueue m_presentQueue;
        vulkan::Capsule<VkSwapchainKHR> m_swapChain{m_device, vkDestroySwapchainKHR};
        std::vector<VkImage> m_swapChainImages;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;
        std::vector<vulkan::Capsule<VkImageView>> m_swapChainImageViews;

        // Graphics pipeline
        vulkan::Capsule<VkPipelineLayout> m_pipelineLayout{m_device, vkDestroyPipelineLayout};
    };
}
