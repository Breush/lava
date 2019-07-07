#pragma once

#include <lava/magma/render-engine.hpp>

#include <lava/magma/render-scenes/render-scene.hpp>
#include <lava/magma/render-targets/i-render-target.hpp>

#include "../vr-engine.hpp"
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
        Impl(RenderEngine& engine);
        ~Impl();

        // Main interface
        void update();
        void draw();
        uint32_t registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath);
        uint32_t addView(RenderImage renderImage, IRenderTarget& renderTarget, Viewport viewport);
        void removeView(uint32_t viewId);
        void logTrackingOnce() { m_logTracking = true; }

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
        void add(std::unique_ptr<RenderScene>&& renderScene);
        void add(std::unique_ptr<IRenderTarget>&& renderTarget);
        /// @}

        /**
         * @name VR
         */
        /// @{
        std::optional<VrEvent> vrPollEvent() { return m_vrEngine.pollEvent(); }
        bool vrEnabled() const { return m_vrEngine.enabled(); }
        bool vrDeviceValid(VrDeviceType deviceType) const { return m_vrEngine.deviceValid(deviceType); }
        const glm::mat4& vrDeviceTransform(VrDeviceType deviceType) const { return m_vrEngine.deviceTransform(deviceType); }
        Mesh& vrDeviceMesh(VrDeviceType deviceType, RenderScene& scene) { return m_vrEngine.deviceMesh(deviceType, scene); }
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
        /// @}

        /**
         * @name Internal interface
         */
        /// @{
        VrEngine& vrEngine() { return m_vrEngine; }
        const VrEngine& vrEngine() const { return m_vrEngine; }

        const MaterialInfo& materialInfo(const std::string& hrid) const;

        void updateRenderViews(RenderImage renderImage);
        /// @}

    protected:
        void initVr();
        void initVulkan();
        void initVulkanDevice(vk::SurfaceKHR* pSurface);
        void initRenderScenes();

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
            std::vector<vk::CommandBuffer> commandBuffers;
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
        $attribute(vulkan::CommandPool, commandPool, {device()});
        $attribute(vulkan::CommandPool, transferCommandPool, {device()});

        /// Shaders
        ShadersManager m_shadersManager{device()};
        std::unordered_map<std::string, MaterialInfo> m_materialInfos;
        std::unordered_map<std::string, uint32_t> m_registeredMaterialsMap;
        chamber::FileWatcher m_shadersWatcher;

        /// VR
        VrEngine m_vrEngine;

        /**
         * @name Textures
         */
        /// @{
        /// Dummy texture for colors. 1x1 pixel of rgba(255, 255, 255, 255)
        vulkan::ImageHolder m_dummyImageHolder{*this, "magma.vulkan.render-engine.dummy-image"};

        /// Dummy texture for normal mapping. 1x1 pixel of rgba(128, 128, 255, 255)
        vulkan::ImageHolder m_dummyNormalImageHolder{*this, "magma.vulkan.render-engine.dummy-normal-image"};

        /// Dummy texture for black-invisible. 1x1 pixel of rgba(0, 0, 0, 0)
        vulkan::ImageHolder m_dummyInvisibleImageHolder{*this, "magma.vulkan.render-engine.dummy-invisible-image"};

        /// Dummy texture for cube maps. 1x1 pixel of rgba(255, 255, 255, 255)
        vulkan::ImageHolder m_dummyCubeImageHolder{*this, "magma.vulkan.render-engine.dummy-cube-image"};

        /// Dummy texture sampler.
        $attribute(vulkan::Sampler, dummySampler, {device()});

        /// Shadow-map sampler.
        $attribute(vulkan::Sampler, shadowsSampler, {device()});
        /// @}

        // Data
        std::vector<std::unique_ptr<RenderScene>> m_renderScenes;
        std::vector<RenderTargetBundle> m_renderTargetBundles;
        std::vector<RenderView> m_renderViews;
    };
}
