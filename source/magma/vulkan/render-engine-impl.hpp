#pragma once

#include <glm/mat4x4.hpp>
#include <lava/chamber/macros.hpp>
#include <lava/crater/window.hpp>
#include <lava/magma/interfaces/camera.hpp>
#include <lava/magma/interfaces/material.hpp>
#include <lava/magma/interfaces/mesh.hpp>
#include <lava/magma/interfaces/point-light.hpp>
#include <lava/magma/render-engine.hpp>

#include "./device.hpp"
#include "./instance.hpp"
#include "./surface.hpp"
#include "./swapchain.hpp"

// @todo Move inside RenderEngine::Impl and rename accordingly
struct UniformBufferObject {
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    glm::vec4 pointLightPosition;
};

namespace lava::magma {
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

        /**
         * @name Adders
         */
        /// @{
        void add(std::unique_ptr<ICamera>&& camera) { m_cameras.emplace_back(std::move(camera)); }
        void add(std::unique_ptr<IMaterial>&& material) { m_materials.emplace_back(std::move(material)); }
        void add(std::unique_ptr<IMesh>&& mesh) { m_meshes.emplace_back(std::move(mesh)); }
        void add(std::unique_ptr<IPointLight>&& pointLight) { m_pointLights.emplace_back(std::move(pointLight)); }
        void add(IRenderTarget& renderTarget);
        /// @}

    protected:
        void createRenderPass();
        void createGraphicsPipeline();

        void createFramebuffers();
        void createCommandPool();
        void createDummyTexture();
        void createTextureSampler();
        void createCommandBuffers();

        void createDepthResources();
        void createSemaphores();

        // Transform UBOs
        void createUniformBuffer();
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSet();

    public:
        crater::WindowHandle m_windowHandle;
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

        // Descriptor pools
        $attribute(vulkan::Capsule<VkDescriptorSetLayout>, meshDescriptorSetLayout,
                   {m_device.capsule(), vkDestroyDescriptorSetLayout});
        $attribute(vulkan::Capsule<VkDescriptorPool>, meshDescriptorPool, {m_device.capsule(), vkDestroyDescriptorPool});

        // Graphics pipeline
        $attribute(vulkan::Capsule<VkPipelineLayout>, pipelineLayout, {m_device.capsule(), vkDestroyPipelineLayout});
        $attribute(vulkan::Capsule<VkRenderPass>, renderPass, {m_device.capsule(), vkDestroyRenderPass});
        $attribute(vulkan::Capsule<VkPipeline>, graphicsPipeline, {m_device.capsule(), vkDestroyPipeline});

        // Drawing
        $attribute(std::vector<vulkan::Capsule<VkFramebuffer>>, swapchainFramebuffers);
        $attribute(vulkan::Capsule<VkCommandPool>, commandPool, {m_device.capsule(), vkDestroyCommandPool});
        $attribute(std::vector<VkCommandBuffer>, commandBuffers);

        // Dummy texture
        vulkan::Capsule<VkImage> m_dummyImage{m_device.capsule(), vkDestroyImage};
        vulkan::Capsule<VkDeviceMemory> m_dummyImageMemory{m_device.capsule(), vkFreeMemory};
        $attribute(vulkan::Capsule<VkImageView>, dummyImageView, {m_device.capsule(), vkDestroyImageView});
        $attribute(vulkan::Capsule<VkSampler>, textureSampler, {m_device.capsule(), vkDestroySampler});

        // Depth
        vulkan::Capsule<VkImage> m_depthImage{m_device.capsule(), vkDestroyImage};
        vulkan::Capsule<VkDeviceMemory> m_depthImageMemory{m_device.capsule(), vkFreeMemory};
        vulkan::Capsule<VkImageView> m_depthImageView{m_device.capsule(), vkDestroyImageView};

        // Rendering
        vulkan::Capsule<VkSemaphore> m_imageAvailableSemaphore{m_device.capsule(), vkDestroySemaphore};
        vulkan::Capsule<VkSemaphore> m_renderFinishedSemaphore{m_device.capsule(), vkDestroySemaphore};

        // Transform UBOs
        vulkan::Capsule<VkBuffer> m_uniformStagingBuffer{m_device.capsule(), vkDestroyBuffer};
        vulkan::Capsule<VkDeviceMemory> m_uniformStagingBufferMemory{m_device.capsule(), vkFreeMemory};
        vulkan::Capsule<VkBuffer> m_uniformBuffer{m_device.capsule(), vkDestroyBuffer};
        vulkan::Capsule<VkDeviceMemory> m_uniformBufferMemory{m_device.capsule(), vkFreeMemory};

        // Data
        std::vector<std::unique_ptr<ICamera>> m_cameras;
        std::vector<std::unique_ptr<IMaterial>> m_materials;
        std::vector<std::unique_ptr<IMesh>> m_meshes;
        std::vector<std::unique_ptr<IPointLight>> m_pointLights;
        std::vector<IRenderTarget*> m_renderTargets;
    };
}
