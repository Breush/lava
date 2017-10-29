#pragma once

#include <lava/magma/render-engine.hpp>

#include <glm/mat4x4.hpp>
#include <lava/chamber/macros.hpp>
#include <lava/crater/window.hpp>
#include <lava/magma/render-scenes/i-render-scene.hpp>
#include <lava/magma/render-targets/i-render-target.hpp>

#include "./holders/buffer-holder.hpp"
#include "./holders/device-holder.hpp"
#include "./holders/image-holder.hpp"
#include "./holders/instance-holder.hpp"
#include "./material-info.hpp"
#include "./shaders-manager.hpp"
#include "./wrappers.hpp"

namespace lava::magma {
    class Present;
}

namespace lava::magma {
    /**
     * Vulkan-based implementation of the lava::RenderEngine.
     */
    class RenderEngine::Impl {
    public:
        Impl();
        ~Impl();

        // Main interface
        void stop();
        void draw();
        uint32_t registerMaterial(const std::string& hrid, const std::string& shaderImplementation);
        uint32_t registerMaterial(const std::string& hrid, const std::string& shaderImplementation,
                                  const UniformDefinitions& uniformDefinitions);
        uint32_t addView(ICamera& camera, IRenderTarget& renderTarget, Viewport viewport);

        /**
         * @name Materials
         */
        /// @{
        uint32_t materialId(const std::string& hrid) const { return m_registeredMaterialsMap.at(hrid); }
        /// @}

        /**
         * @name Adders
         */
        /// @{
        void add(std::unique_ptr<IRenderScene>&& renderScene);
        void add(std::unique_ptr<IRenderTarget>&& renderTarget);
        /// @}

        /**
         * @name Getters
         */
        /// @{
        const vulkan::DeviceHolder& deviceHolder() const { return m_deviceHolder; }
        const vk::Instance& instance() const { return m_instanceHolder.instance(); }
        const vk::Device& device() const { return m_deviceHolder.device(); }
        const vk::PhysicalDevice& physicalDevice() const { return m_deviceHolder.physicalDevice(); }
        const vk::Queue& graphicsQueue() const { return m_deviceHolder.graphicsQueue(); }
        const vk::Queue& presentQueue() const { return m_deviceHolder.presentQueue(); }

        ShadersManager& shadersManager() { return m_shadersManager; }
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
        const MaterialInfo& materialInfo(const std::string& hrid) const;

        void updateRenderTarget(uint32_t renderTargetId);
        void updateView(ICamera& camera);
        /// @}

    protected:
        void initVulkan();
        void initVulkanDevice(vk::SurfaceKHR surface);
        void initRenderScenes();

        // Resources
        void createSemaphores();
        void createDummyTextures();

        // Command buffers
        vk::CommandBuffer& recordCommandBuffer(uint32_t renderTargetIndex, uint32_t bufferIndex);
        void createCommandPool(vk::SurfaceKHR surface);
        void createCommandBuffers(uint32_t renderTargetIndex);

    private:
        /// This bundle is what each render target needs.
        struct RenderTargetBundle {
            std::unique_ptr<IRenderTarget> renderTarget;
            std::unique_ptr<Present> presentStage;
            std::vector<vk::CommandBuffer> commandBuffers;
        };

        /// A view bind a scene and a target.
        struct RenderView {
            ICamera* camera = nullptr;
            uint32_t renderTargetId = -1u;
            uint32_t presentViewId = -1u;
        };

    private:
        vulkan::InstanceHolder m_instanceHolder;
        vulkan::DeviceHolder m_deviceHolder;

        // Commands
        $attribute(vulkan::CommandPool, commandPool, {device()});

        /// Shaders.
        ShadersManager m_shadersManager{device()};
        std::unordered_map<std::string, MaterialInfo> m_materialInfos;
        std::unordered_map<std::string, uint32_t> m_registeredMaterialsMap;

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
        std::vector<std::unique_ptr<IRenderScene>> m_renderScenes;
        std::vector<RenderTargetBundle> m_renderTargetBundles;
        std::vector<RenderView> m_renderViews;
    };
}
