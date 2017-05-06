#pragma once

#include <lava/crater/Window.hpp>

#include "./device.hpp"
#include "./instance.hpp"
#include "./surface.hpp"

namespace lava::priv {
    /**
     * Vulkan-based implementation of the lava::Engine.
     */
    class EngineImpl {
    public:
        EngineImpl(lava::Window& window);
        virtual ~EngineImpl();

        void draw();

    protected:
        void initVulkan();

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

        vulkan::Instance m_instance;
        vulkan::Device m_device;
        vulkan::Surface m_surface{m_instance};

        // Swap chain
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
