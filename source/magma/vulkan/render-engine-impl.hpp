#pragma once

#include <glm/mat4x4.hpp>
#include <lava/chamber/macros.hpp>
#include <lava/crater/window.hpp>
#include <lava/magma/interfaces/camera.hpp>
#include <lava/magma/interfaces/material.hpp>
#include <lava/magma/interfaces/mesh.hpp>
#include <lava/magma/interfaces/point-light.hpp>
#include <lava/magma/interfaces/render-target.hpp>
#include <lava/magma/render-engine.hpp>

#include "./device-holder.hpp"
#include "./instance-holder.hpp"
#include "./render-target-data.hpp"
#include "./stages.hpp"
#include "./wrappers.hpp"

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
        void add(std::unique_ptr<IRenderTarget>&& renderTarget);
        /// @}

        /**
         * @name Getters
         */
        /// @{
        const vk::Instance& instance() const { return m_instanceHolder.instance(); }
        const vk::Device& device() const { return m_deviceHolder.device(); }
        const vk::PhysicalDevice& physicalDevice() const { return m_deviceHolder.physicalDevice(); }
        const vk::Queue& graphicsQueue() const { return m_deviceHolder.graphicsQueue(); }
        const vk::Queue& presentQueue() const { return m_deviceHolder.presentQueue(); }
        /// @}

        /**
         * @name Textures
         */
        /// @{
        vk::ImageView dummyImageView() const { return m_dummyImageHolder.view(); }
        vk::ImageView dummyNormalImageView() const { return m_dummyNormalImageHolder.view(); }
        /// @}

        /**
         * @name Internal interface
         */
        /// @{
        const ICamera& camera(uint32_t index) const { return *m_cameras[index]; }
        const IMaterial& material(uint32_t index) const { return *m_materials[index]; }
        const IMesh& mesh(uint32_t index) const { return *m_meshes[index]; }
        const IPointLight& pointLight(uint32_t index) const { return *m_pointLights[index]; }

        const std::vector<std::unique_ptr<ICamera>>& cameras() const { return m_cameras; }
        const std::vector<std::unique_ptr<IMaterial>>& materials() const { return m_materials; }
        const std::vector<std::unique_ptr<IMesh>>& meshes() const { return m_meshes; }
        const std::vector<std::unique_ptr<IPointLight>>& pointLights() const { return m_pointLights; }
        /// @}

    protected:
        void initVulkan();
        void initVulkanDevice(vk::SurfaceKHR surface);

        // Pipelines
        void initStages();
        void updateStages();
        void createSemaphores();

        // Dummies
        void createDummyTextures();

        // Command buffers
        vk::CommandBuffer& recordCommandBuffer(uint32_t renderTargetIndex, uint32_t bufferIndex);
        void createCommandPool(vk::SurfaceKHR surface);
        void createCommandBuffers(uint32_t renderTargetIndex);

        // Transform UBOs
        void createDescriptorSetLayouts();
        void createDescriptorPool();

    private:
        /// This bundle is what each render target needs.
        struct RenderTargetBundle {
            std::unique_ptr<IRenderTarget> renderTarget;
            std::unique_ptr<Present> presentStage;
            std::vector<vk::CommandBuffer> commandBuffers;

            inline const DataRenderTarget& data() { return *reinterpret_cast<const DataRenderTarget*>(renderTarget->data()); }
        };

    private:
        vulkan::InstanceHolder m_instanceHolder;
        vulkan::DeviceHolder m_deviceHolder;

        // Descriptor layouts and pools
        $attribute(vulkan::DescriptorSetLayout, cameraDescriptorSetLayout, {device()});
        $attribute(vulkan::DescriptorPool, cameraDescriptorPool, {device()});
        $attribute(vulkan::DescriptorSetLayout, meshDescriptorSetLayout, {device()});
        $attribute(vulkan::DescriptorPool, meshDescriptorPool, {device()});
        $attribute(vulkan::DescriptorSetLayout, materialDescriptorSetLayout, {device()});
        $attribute(vulkan::DescriptorPool, materialDescriptorPool, {device()});

        // Rendering stages
        // @note We currently have only one scene, so these are standalone here.
        GBuffer m_gBuffer{*this};
        Epiphany m_epiphany{*this};

        // Commands
        $attribute(vulkan::CommandPool, commandPool, {device()});

        /**
         * @name Textures
         */
        /// @{
        /// Dummy texture for colors. 1x1 pixel of rgba(255, 255, 255, 255)
        vulkan::ImageHolder m_dummyImageHolder{*this};

        /// Dummy texture for normal mapping. 1x1 pixel of rgba(128, 128, 255, 255)
        vulkan::ImageHolder m_dummyNormalImageHolder{*this};

        /// Dummy texture sampler.
        $attribute(vulkan::Sampler, dummySampler, {device()});
        /// @}

        // Semaphores
        vulkan::Semaphore m_renderFinishedSemaphore{device()};

        // Data
        std::vector<std::unique_ptr<ICamera>> m_cameras;
        std::vector<std::unique_ptr<IMaterial>> m_materials;
        std::vector<std::unique_ptr<IMesh>> m_meshes;
        std::vector<std::unique_ptr<IPointLight>> m_pointLights;
        std::vector<RenderTargetBundle> m_renderTargetBundles;
    };
}
