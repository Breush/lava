#pragma once

#include <glm/mat4x4.hpp>
#include <lava/chamber/properties.hpp>
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

        $property_readonly(vulkan::Instance, instance);
        $property_readonly(vulkan::Surface, surface, {m_instance});
        $property_readonly(vulkan::Device, device);
        $property_readonly(vulkan::Swapchain, swapchain, {m_device});

        // UBO
        $property_readonly(vulkan::Capsule<VkDescriptorSetLayout>, descriptorSetLayout,
                           {m_device.capsule(), vkDestroyDescriptorSetLayout});
        $property_readonly(vulkan::Capsule<VkDescriptorPool>, descriptorPool, {m_device.capsule(), vkDestroyDescriptorPool});
        $property_readonly(VkDescriptorSet, descriptorSet);

        // Graphics pipeline
        $property_readonly(vulkan::Capsule<VkPipelineLayout>, pipelineLayout, {m_device.capsule(), vkDestroyPipelineLayout});
        $property_readonly(vulkan::Capsule<VkRenderPass>, renderPass, {m_device.capsule(), vkDestroyRenderPass});
        $property_readonly(vulkan::Capsule<VkPipeline>, graphicsPipeline, {m_device.capsule(), vkDestroyPipeline});

        // Drawing
        $property_readonly(std::vector<vulkan::Capsule<VkFramebuffer>>, swapchainFramebuffers);
        $property_readonly(vulkan::Capsule<VkCommandPool>, commandPool, {m_device.capsule(), vkDestroyCommandPool});
        $property_readonly(std::vector<VkCommandBuffer>, commandBuffers);

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
