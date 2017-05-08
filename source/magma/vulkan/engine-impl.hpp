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

        $attribute(vulkan::Instance, instance);
        $attribute(vulkan::Surface, surface, {m_instance});
        $attribute(vulkan::Device, device);
        $attribute(vulkan::Swapchain, swapchain, {m_device});

        // UBO
        $attribute(vulkan::Capsule<VkDescriptorSetLayout>, descriptorSetLayout,
                   {m_device.capsule(), vkDestroyDescriptorSetLayout});
        $attribute(vulkan::Capsule<VkDescriptorPool>, descriptorPool, {m_device.capsule(), vkDestroyDescriptorPool});
        $attribute(VkDescriptorSet, descriptorSet);

        // Graphics pipeline
        $attribute(vulkan::Capsule<VkPipelineLayout>, pipelineLayout, {m_device.capsule(), vkDestroyPipelineLayout});
        $attribute(vulkan::Capsule<VkRenderPass>, renderPass, {m_device.capsule(), vkDestroyRenderPass});
        $attribute(vulkan::Capsule<VkPipeline>, graphicsPipeline, {m_device.capsule(), vkDestroyPipeline});

        // Drawing
        $attribute(std::vector<vulkan::Capsule<VkFramebuffer>>, swapchainFramebuffers);
        $attribute(vulkan::Capsule<VkCommandPool>, commandPool, {m_device.capsule(), vkDestroyCommandPool});
        $attribute(std::vector<VkCommandBuffer>, commandBuffers);

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
