#pragma once

#include <lava/magma/render-engine.hpp>

#include <lava/magma/render-targets/i-render-target.hpp>
#include <lava/magma/scene.hpp>

#include "./holders/buffer-holder.hpp"
#include "./holders/device-holder.hpp"
#include "./holders/image-holder.hpp"
#include "./holders/instance-holder.hpp"
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
        Impl(RenderEngine& engine);
        ~Impl();

        // Main interface
        void update();
        void draw();
        uint32_t registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath);
        uint32_t addView(RenderImage renderImage, IRenderTarget& renderTarget, const Viewport& viewport);
        void removeView(uint32_t viewId);
        void logTrackingOnce() { m_logTracking = true; }

        /**
         * @name Materials
         */
        /// @{
        const MaterialInfo& materialInfo(const std::string& hrid) const;
        const MaterialInfo* materialInfoIfExists(const std::string& hrid) const;
        uint32_t materialId(const std::string& hrid) const { return m_materialInfos.at(hrid).id; }
        /// @}

        /**
         * @name Adders
         */
        /// @{
        void add(Scene& scene);
        void add(std::unique_ptr<IRenderTarget>&& renderTarget);
        /// @}

        /**
         * @name Getters
         */
        /// @{
        RenderEngine& engine() { return m_engine; }
        const RenderEngine& engine() const { return m_engine; }

        const vulkan::DeviceHolder& deviceHolder() const { return m_deviceHolder; }
        const vk::Instance& instance() const { return m_instanceHolder.instance(); }
        const vk::Device& device() const { return m_deviceHolder.device(); }
        const vk::PhysicalDevice& physicalDevice() const { return m_deviceHolder.physicalDevice(); }
        const vk::Queue& graphicsQueue() const { return m_deviceHolder.graphicsQueue(); }
        const vk::Queue& transferQueue() const { return m_deviceHolder.transferQueue(); }
        const vk::Queue& presentQueue() const { return m_deviceHolder.presentQueue(); }
        uint32_t graphicsQueueFamilyIndex() const { return m_deviceHolder.graphicsQueueFamilyIndex(); }
        uint32_t presentQueueFamilyIndex() const { return m_deviceHolder.presentQueueFamilyIndex(); }

        vk::CommandPool commandPool() const { return m_commandPool.get(); }
        vk::CommandPool transferCommandPool() const { return m_transferCommandPool.get(); }

        ShadersManager& shadersManager() { return m_shadersManager; }
        /// @}

        /**
         * @name Textures
         */
        /// @{
        vk::ImageView dummyImageView() const { return m_dummyImageHolder.view(); }
        vk::ImageView dummyNormalImageView() const { return m_dummyNormalImageHolder.view(); }
        vk::ImageView dummyInvisibleImageView() const { return m_dummyInvisibleImageHolder.view(); }
        vk::ImageView dummyCubeImageView() const { return m_dummyCubeImageHolder.view(); }

        vk::Sampler dummySampler() const { return m_dummySampler.get(); }
        vk::Sampler shadowsSampler() const { return m_shadowsSampler.get(); }
        /// @}

        /**
         * @name Internal interface
         */
        /// @{
        void updateRenderViews(RenderImage renderImage);
        /// @}

    protected:
        void initVr();
        void initVulkan();
        void initVulkanDevice(vk::SurfaceKHR* pSurface);
        void initScenes();

        void updateVr();
        void updateShaders();

        // Resources
        void createDummyTextures();

        // Command buffers
        std::vector<vk::CommandBuffer> recordCommandBuffer(uint32_t renderTargetIndex, uint32_t bufferIndex);
        void createCommandPools(vk::SurfaceKHR* pSurface);
        void createCommandBuffers(uint32_t renderTargetIndex);

    private:
        /// This bundle is what each render target needs.
        struct RenderTargetBundle {
            std::unique_ptr<IRenderTarget> renderTarget;
            // @fixme Add a compositor stage for every render targets
            // std::unique_ptr<Present> presentStage;
            std::vector<vk::UniqueCommandBuffer> commandBuffers;
            bool prepareOk = false; //!< Whether we can draw this frame.
        };

        /// A view bind a scene and a target.
        struct RenderView {
            RenderImage renderImage;
            uint32_t renderTargetId = -1u;
            uint32_t presentViewId = -1u;
        };

    private:
        RenderEngine& m_engine;
        bool m_logTracking = false;

        vulkan::InstanceHolder m_instanceHolder;
        vulkan::DeviceHolder m_deviceHolder;

        // Commands
        vk::UniqueCommandPool m_commandPool;
        vk::UniqueCommandPool m_transferCommandPool;

        /// Shaders
        ShadersManager m_shadersManager{device()};
        std::unordered_map<std::string, MaterialInfo> m_materialInfos;
        chamber::FileWatcher m_shadersWatcher;

        /**
         * @name Textures
         */
        /// @{
        /// Dummy texture for colors. 1x1 pixel of rgba(255, 255, 255, 255)
        vulkan::ImageHolder m_dummyImageHolder;

        /// Dummy texture for normal mapping. 1x1 pixel of rgba(128, 128, 255, 255)
        vulkan::ImageHolder m_dummyNormalImageHolder;

        /// Dummy texture for black-invisible. 1x1 pixel of rgba(0, 0, 0, 0)
        vulkan::ImageHolder m_dummyInvisibleImageHolder;

        /// Dummy texture for cube maps. 1x1 pixel of rgba(255, 255, 255, 255)
        vulkan::ImageHolder m_dummyCubeImageHolder;

        /// Dummy texture sampler.
        vk::UniqueSampler m_dummySampler;

        /// Shadow-map sampler.
        vk::UniqueSampler m_shadowsSampler;
        /// @}

        // Data
        std::vector<Scene*> m_scenes;
        std::vector<RenderTargetBundle> m_renderTargetBundles;
        std::vector<RenderView> m_renderViews;
    };
}
