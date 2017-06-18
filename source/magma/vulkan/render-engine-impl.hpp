#pragma once

#include <glm/mat4x4.hpp>
#include <lava/chamber/properties.hpp>
#include <lava/crater/Window.hpp>
#include <lava/magma/mesh.hpp>
#include <lava/magma/render-engine.hpp>

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
     * Vulkan-based implementation of the lava::RenderEngine.
     */
    class RenderEngine::Impl {
    public:
        Impl();
        ~Impl();

        // Main interface
        void draw();
        void update();
        void add(IRenderTarget& renderTarget);

        // Internal interface
        void add(Mesh::Impl& mesh);

    protected:
        void createRenderPass();
        void createGraphicsPipeline();

        void createFramebuffers();
        void createCommandPool();
        void createTextureImage();
        void createTextureImageView();
        void createTextureSampler();
        void createCommandBuffers();

        void createDepthResources();
        void createSemaphores();

        // Mesh
        void createUniformBuffer();
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSet();

        // @todo TBR Temporary setup
    public:
        WindowHandle m_windowHandle;
        VkExtent2D m_windowExtent;

        void initVulkan(); // @todo That is a really really bad idea
        void recreateSwapchain();

    private:
        $attribute(vulkan::Instance, instance);
        $attribute(vulkan::Surface, surface, {m_instance});
        $attribute(vulkan::Device, device);
        $attribute(vulkan::Swapchain, swapchain, {m_device}); // @todo TBR

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

        // Debug texture
        vulkan::Capsule<VkImage> m_textureImage{m_device.capsule(), vkDestroyImage};
        vulkan::Capsule<VkDeviceMemory> m_textureImageMemory{m_device.capsule(), vkFreeMemory};

        $attribute(vulkan::Capsule<VkImageView>, textureImageView, {m_device.capsule(), vkDestroyImageView});
        $attribute(vulkan::Capsule<VkSampler>, textureSampler, {m_device.capsule(), vkDestroySampler});

        // Depth
        vulkan::Capsule<VkImage> m_depthImage{m_device.capsule(), vkDestroyImage};
        vulkan::Capsule<VkDeviceMemory> m_depthImageMemory{m_device.capsule(), vkFreeMemory};
        vulkan::Capsule<VkImageView> m_depthImageView{m_device.capsule(), vkDestroyImageView};

        // Rendering
        vulkan::Capsule<VkSemaphore> m_imageAvailableSemaphore{m_device.capsule(), vkDestroySemaphore};
        vulkan::Capsule<VkSemaphore> m_renderFinishedSemaphore{m_device.capsule(), vkDestroySemaphore};

        // Mesh
        vulkan::Capsule<VkBuffer> m_uniformStagingBuffer{m_device.capsule(), vkDestroyBuffer};
        vulkan::Capsule<VkDeviceMemory> m_uniformStagingBufferMemory{m_device.capsule(), vkFreeMemory};
        vulkan::Capsule<VkBuffer> m_uniformBuffer{m_device.capsule(), vkDestroyBuffer};
        vulkan::Capsule<VkDeviceMemory> m_uniformBufferMemory{m_device.capsule(), vkFreeMemory};

        // Targets
        std::vector<IRenderTarget*> m_renderTargets;

        // Meshes
        std::vector<Mesh::Impl*> m_meshes;
    };
}
