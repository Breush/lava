#pragma once

#include <vulkan/vulkan.hpp>

#include <lava/crater/Window.hpp>

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

        void initSurface(Window& window);

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

        // Surface
        VkSurfaceKHR m_surface;
        VkFormat m_colorFormat;
        VkColorSpaceKHR m_colorSpace;
        uint32_t m_queueNodeIndex = UINT32_MAX;
    };
}
