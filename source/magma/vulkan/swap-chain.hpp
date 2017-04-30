#pragma once

#include <vulkan/vulkan.hpp>

namespace lava::priv {
    class EngineImpl;
}

namespace lava {
    /*
    * Class wrapping access to the swap chain.
    * A swap chain is a collection of framebuffers used for rendering and presentation to the windowing system.
    */
    class SwapChain {
    public:
        void bind(priv::EngineImpl& engine);

    private:
        priv::EngineImpl* m_engine = nullptr;

        PFN_vkGetPhysicalDeviceSurfaceSupportKHR m_fpGetPhysicalDeviceSurfaceSupportKHR;
        PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR m_fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
        PFN_vkGetPhysicalDeviceSurfaceFormatsKHR m_fpGetPhysicalDeviceSurfaceFormatsKHR;
        PFN_vkGetPhysicalDeviceSurfacePresentModesKHR m_fpGetPhysicalDeviceSurfacePresentModesKHR;
        PFN_vkCreateSwapchainKHR m_fpCreateSwapchainKHR;
        PFN_vkDestroySwapchainKHR m_fpDestroySwapchainKHR;
        PFN_vkGetSwapchainImagesKHR m_fpGetSwapchainImagesKHR;
        PFN_vkAcquireNextImageKHR m_fpAcquireNextImageKHR;
        PFN_vkQueuePresentKHR m_fpQueuePresentKHR;
    };
}
