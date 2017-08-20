#pragma once

#include <glm/mat4x4.hpp>
#include <lava/chamber/macros.hpp>
#include <lava/crater/window.hpp>
#include <lava/magma/interfaces/render-scene.hpp>
#include <lava/magma/interfaces/render-target.hpp>
#include <lava/magma/render-engine.hpp>

#include "./holders/device-holder.hpp"
#include "./holders/instance-holder.hpp"
#include "./render-target-data.hpp"
#include "./stages/present.hpp"
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
        uint32_t addView(IRenderScene& renderScene, uint32_t renderSceneCameraIndex, IRenderTarget& renderTarget,
                         Viewport viewport);

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
        void updateRenderTarget(uint32_t renderTargetId);
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
            std::vector<vk::CommandBuffer> commandBuffers;

            inline const DataRenderTarget& data() { return *reinterpret_cast<const DataRenderTarget*>(renderTarget->data()); }
        };

        /// A view bind a scene and a target.
        struct RenderView {
            IRenderScene* renderScene = nullptr;
            IRenderTarget* renderTarget = nullptr;
            std::unique_ptr<Present> presentStage;
        };

    private:
        vulkan::InstanceHolder m_instanceHolder;
        vulkan::DeviceHolder m_deviceHolder;

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
        std::vector<std::unique_ptr<IRenderScene>> m_renderScenes;
        std::vector<RenderTargetBundle> m_renderTargetBundles;
        std::vector<RenderView> m_renderViews;
    };
}
