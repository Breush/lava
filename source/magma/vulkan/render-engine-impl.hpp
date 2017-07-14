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
#include "./render-engine/g-buffer.hpp"
#include "./render-engine/present.hpp"
#include "./surface.hpp"
#include "./swapchain.hpp"

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

        /**
         * @name Internal interface
         */
        /// @{
        const std::vector<std::unique_ptr<ICamera>>& cameras() const { return m_cameras; }
        const std::vector<std::unique_ptr<IMesh>>& meshes() const { return m_meshes; }
        /// @}

    protected:
        // Pipelines
        void initStages();
        void updateStages();
        void createSemaphores();

        // Textures
        void createDummyTexture();
        void createTextureSampler();

        // Command buffers
        VkCommandBuffer& recordCommandBuffer(uint32_t index);
        void createCommandPool();
        void createCommandBuffers();

        // Transform UBOs
        void createDescriptorSetLayouts();
        void createDescriptorPool();

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

        // Descriptor layouts and pools
        $attribute(vulkan::Capsule<VkDescriptorSetLayout>, cameraDescriptorSetLayout,
                   {m_device.capsule(), vkDestroyDescriptorSetLayout});
        $attribute(vulkan::Capsule<VkDescriptorPool>, cameraDescriptorPool, {m_device.capsule(), vkDestroyDescriptorPool});
        $attribute(vulkan::Capsule<VkDescriptorSetLayout>, meshDescriptorSetLayout,
                   {m_device.capsule(), vkDestroyDescriptorSetLayout});
        $attribute(vulkan::Capsule<VkDescriptorPool>, meshDescriptorPool, {m_device.capsule(), vkDestroyDescriptorPool});
        $attribute(vulkan::Capsule<VkDescriptorSetLayout>, materialDescriptorSetLayout,
                   {m_device.capsule(), vkDestroyDescriptorSetLayout});
        $attribute(vulkan::Capsule<VkDescriptorPool>, materialDescriptorPool, {m_device.capsule(), vkDestroyDescriptorPool});

        // Rendering
        GBuffer m_gBuffer{*this};
        Present m_present{*this};

        // Commands
        $attribute(vulkan::Capsule<VkCommandPool>, commandPool, {m_device.capsule(), vkDestroyCommandPool});
        $attribute(std::vector<VkCommandBuffer>, commandBuffers);

        /// Dummy texture for colors. 1x1 pixel of rgba(255, 255, 255, 255)
        vulkan::Capsule<VkImage> m_dummyImage{m_device.capsule(), vkDestroyImage};
        vulkan::Capsule<VkDeviceMemory> m_dummyImageMemory{m_device.capsule(), vkFreeMemory};
        $attribute(vulkan::Capsule<VkImageView>, dummyImageView, {m_device.capsule(), vkDestroyImageView});

        /// Dummy texture for normal mapping. 1x1 pixel of rgba(128, 128, 255, 255)
        vulkan::Capsule<VkImage> m_dummyNormalImage{m_device.capsule(), vkDestroyImage};
        vulkan::Capsule<VkDeviceMemory> m_dummyNormalImageMemory{m_device.capsule(), vkFreeMemory};
        $attribute(vulkan::Capsule<VkImageView>, dummyNormalImageView, {m_device.capsule(), vkDestroyImageView});

        $attribute(vulkan::Capsule<VkSampler>, textureSampler, {m_device.capsule(), vkDestroySampler});

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
