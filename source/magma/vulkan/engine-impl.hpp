#pragma once

#include <glm/mat4x4.hpp>
#include <lava/crater/Window.hpp>
#include <lava/magma/engine.hpp>
#include <lava/magma/mesh.hpp>

#include "./device.hpp"
#include "./instance.hpp"
#include "./surface.hpp"
#include "./swapchain.hpp"

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

namespace lava {
    /**
     * Vulkan-based implementation of the lava::Engine.
     */
    class Engine::Impl {
    public:
        Impl(lava::Window& window);
        ~Impl();

        // Main interface
        void draw();
        void update();

        void mode(const lava::VideoMode& mode);

        // Internal interface
        void add(Mesh::Impl& mesh);

        // Getters
        // $property::readOnly(vulkan::Device, device);

        vulkan::Device& device() { return m_device; }
        vulkan::Swapchain& swapchain() { return m_swapchain; }
        vulkan::Capsule<VkDescriptorSetLayout>& descriptorSetLayout() { return m_descriptorSetLayout; }
        vulkan::Capsule<VkDescriptorPool>& descriptorPool() { return m_descriptorPool; }
        vulkan::Capsule<VkCommandPool>& commandPool() { return m_commandPool; }
        vulkan::Capsule<VkPipelineLayout>& pipelineLayout() { return m_pipelineLayout; }

    protected:
        void initVulkan();

        void createRenderPass();
        void createGraphicsPipeline();

        void createDescriptorSetLayout();

        void createFramebuffers();
        void createCommandPool();
        void createCommandBuffers();

        void createSemaphores();

        void recreateSwapchain();

        // Mesh
        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffer();
        void createDescriptorPool();
        void createDescriptorSet();

    private:
        lava::WindowHandle m_windowHandle;
        VkExtent2D m_windowExtent;

        vulkan::Instance m_instance;
        vulkan::Device m_device;
        vulkan::Surface m_surface{m_instance};
        vulkan::Swapchain m_swapchain{m_device};

        // UBO
        vulkan::Capsule<VkDescriptorSetLayout> m_descriptorSetLayout{m_device.capsule(), vkDestroyDescriptorSetLayout};
        vulkan::Capsule<VkDescriptorPool> m_descriptorPool{m_device.capsule(), vkDestroyDescriptorPool};
        VkDescriptorSet m_descriptorSet;

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

        vulkan::Capsule<VkBuffer> m_uniformStagingBuffer{m_device.capsule(), vkDestroyBuffer};
        vulkan::Capsule<VkDeviceMemory> m_uniformStagingBufferMemory{m_device.capsule(), vkFreeMemory};
        vulkan::Capsule<VkBuffer> m_uniformBuffer{m_device.capsule(), vkDestroyBuffer};
        vulkan::Capsule<VkDeviceMemory> m_uniformBufferMemory{m_device.capsule(), vkFreeMemory};

        // Meshes
        std::vector<Mesh::Impl*> m_meshes;
    };
}
