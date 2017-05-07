#pragma once

#include <lava/crater/Window.hpp>

#include "./device.hpp"
#include "./instance.hpp"
#include "./surface.hpp"
#include "./swapchain.hpp"

namespace lava::priv {
    /**
     * Vulkan-based implementation of the lava::Engine.
     */
    class EngineImpl {
    public:
        EngineImpl(lava::Window& window);
        virtual ~EngineImpl();

        void draw();
        void mode(const lava::VideoMode& mode);

    protected:
        void initVulkan();

        void createRenderPass();
        void createGraphicsPipeline();

        void createFramebuffers();
        void createCommandPool();
        void createCommandBuffers();

        void createSemaphores();

        void recreateSwapchain();

        // Mesh
        void createVertexBuffer();
        void createIndexBuffer();

    private:
        lava::WindowHandle m_windowHandle;
        VkExtent2D m_windowExtent;

        vulkan::Instance m_instance;
        vulkan::Device m_device;
        vulkan::Surface m_surface{m_instance};
        vulkan::Swapchain m_swapchain{m_device};

        // Swap chain

        // Graphics pipeline
        vulkan::Capsule<VkPipelineLayout> m_pipelineLayout{m_device.capsule(), vkDestroyPipelineLayout};
        vulkan::Capsule<VkRenderPass> m_renderPass{m_device.capsule(), vkDestroyRenderPass};
        vulkan::Capsule<VkPipeline> m_graphicsPipeline{m_device.capsule(), vkDestroyPipeline};

        // Drawing
        std::vector<vulkan::Capsule<VkFramebuffer>> m_swapchainFramebuffers;
        vulkan::Capsule<VkCommandPool> m_commandPool{m_device.capsule(), vkDestroyCommandPool};
        std::vector<VkCommandBuffer> m_commandBuffers;

        // Rendering
        vulkan::Capsule<VkSemaphore> m_imageAvailableSemaphore{m_device.capsule(), vkDestroySemaphore};
        vulkan::Capsule<VkSemaphore> m_renderFinishedSemaphore{m_device.capsule(), vkDestroySemaphore};

        // Mesh
        vulkan::Capsule<VkBuffer> m_vertexBuffer{m_device.capsule(), vkDestroyBuffer};
        vulkan::Capsule<VkDeviceMemory> m_vertexBufferMemory{m_device.capsule(), vkFreeMemory};
        vulkan::Capsule<VkBuffer> m_indexBuffer{m_device.capsule(), vkDestroyBuffer};
        vulkan::Capsule<VkDeviceMemory> m_indexBufferMemory{m_device.capsule(), vkFreeMemory};
    };
}
