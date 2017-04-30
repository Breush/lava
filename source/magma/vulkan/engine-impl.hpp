#pragma once

#include "./device.hpp"
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

        inline VkInstance& instance() { return m_instance; }
        inline lava::Device& device() { return m_device; }

    protected:
        VkResult vulkanCreateInstance();
        void initVulkan();

    private:
        VkInstance m_instance;
        VkQueue m_queue;
        lava::Device m_device;
        lava::SwapChain m_swapChain;
        lava::Semaphores m_semaphores;

        VkSubmitInfo m_submitInfo;
        VkPipelineStageFlags m_submitPipelineStages;
        VkPhysicalDeviceFeatures m_enabledFeatures;
        std::vector<const char*> m_enabledExtensions;
    };
}
