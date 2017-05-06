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
        virtual ~EngineImpl();

        inline vulkan::Capsule<VkInstance>& instance() { return m_instance; }

        void draw();

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

        void createRenderPass();
        void createGraphicsPipeline();

        void createFramebuffers();
        void createCommandPool();
        void createCommandBuffers();

        void createSemaphores();

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
        vulkan::Device m_device;

        // Surfaces (and swap chain)
        vulkan::Capsule<VkSurfaceKHR> m_surface{m_instance, vkDestroySurfaceKHR};
        vulkan::Capsule<VkSwapchainKHR> m_swapChain{m_device.capsule(), vkDestroySwapchainKHR};
        std::vector<VkImage> m_swapChainImages;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;
        std::vector<vulkan::Capsule<VkImageView>> m_swapChainImageViews;

        // Graphics pipeline
        vulkan::Capsule<VkPipelineLayout> m_pipelineLayout{m_device.capsule(), vkDestroyPipelineLayout};
        vulkan::Capsule<VkRenderPass> m_renderPass{m_device.capsule(), vkDestroyRenderPass};
        vulkan::Capsule<VkPipeline> m_graphicsPipeline{m_device.capsule(), vkDestroyPipeline};

        // Drawing
        std::vector<vulkan::Capsule<VkFramebuffer>> m_swapChainFramebuffers;
        vulkan::Capsule<VkCommandPool> m_commandPool{m_device.capsule(), vkDestroyCommandPool};
        std::vector<VkCommandBuffer> m_commandBuffers;

        // Rendering
        vulkan::Capsule<VkSemaphore> m_imageAvailableSemaphore{m_device.capsule(), vkDestroySemaphore};
        vulkan::Capsule<VkSemaphore> m_renderFinishedSemaphore{m_device.capsule(), vkDestroySemaphore};
    };
}
