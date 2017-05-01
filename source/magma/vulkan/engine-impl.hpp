#pragma once

#include "./capsule.hpp"
#include "./device.hpp"
#include "./proxy.hpp"
#include "./swap-chain.hpp"

namespace lava {
    struct Semaphores {
        VkSemaphore presentComplete;
        VkSemaphore renderComplete;
        VkSemaphore textOverlayComplete;
    };
}

namespace lava::priv {
    /**
     * Vulkan-based implementation of the lava::Engine.
     */
    class EngineImpl {
    public:
        EngineImpl();

        inline vulkan::Capsule<VkInstance>& instance() { return m_instance; }
        inline lava::Device& device() { return m_device; }
        inline lava::SwapChain& swapChain() { return m_swapChain; }

    protected:
        void initVulkan();
        void createInstance();
        void setupDebug();

        void initApplication(VkInstanceCreateInfo& instanceCreateInfo);
        void initValidationLayers(VkInstanceCreateInfo& instanceCreateInfo);
        void initRequiredExtensions(VkInstanceCreateInfo& instanceCreateInfo);

        void pickPhysicalDevice();

    private:
        // VkQueue m_queue;
        lava::Device m_device;
        lava::SwapChain m_swapChain;
        /*lava::Semaphores m_semaphores;

        VkSubmitInfo m_submitInfo;
        VkPipelineStageFlags m_submitPipelineStages;
        VkPhysicalDeviceFeatures m_enabledFeatures;
        std::vector<const char*> m_enabledExtensions;*/

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
    };
}
