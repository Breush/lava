#pragma once

#include <lava/magma/render-image.hpp>

#include "../vulkan/command-buffer-thread.hpp"
#include "../vulkan/environment.hpp"
#include "../vulkan/holders/descriptor-holder.hpp"
#include "../vulkan/shadows.hpp"

namespace lava::magma {
    class Scene;
    class Light;
    class Camera;
    class Material;
    class Texture;
    class Mesh;
    class ShadowsStage;
}

namespace lava::magma {
    class SceneAft {
    public:
        SceneAft(Scene& scene, RenderEngine& engine);

        void init();
        void update();

        /**
         * @name Record
         */
        /// @{
        void record();
        void waitRecord();

        /// All recorded command buffers during last record() call.
        const std::vector<vk::CommandBuffer>& commandBuffers() const { return m_commandBuffers; }
        /// @}

        /**
         * @name Ids
         *
         * Unique ids across all scenes.
         */
        /// @{
        uint16_t cameraId(const Camera& camera) const { return m_cameraBundles.at(&camera).id; }
        uint16_t lightId(const Light& light) const { return m_lightBundles.at(&light).id; }
        /// @}

        /**
         * @name Cameras API
         */
        /// @{
        RenderImage cameraRenderImage(const Camera& camera) const;
        RenderImage cameraDepthRenderImage(const Camera& camera) const;

        void updateCamera(const Camera& camera);
        void changeCameraRenderImageLayout(const Camera& camera, vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer);
        /// @}

        /**
         * @name Shadows
         */
        /// @{
        const Shadows& shadows(const Light& light, const Camera& camera) const;
        RenderImage shadowsCascadeRenderImage(const Light& light, const Camera* camera = nullptr,
                                              uint32_t cascadeIndex = 0u) const;

        /**
         * Says to use the fallbackCamera's shadows for the specified camera.
         * This is used for VR rendering in order to share the shadow maps between the two eyes.
         */
        void shadowsFallbackCamera(Camera& camera, const Camera& fallbackCamera);
        float shadowsCascadeSplitDepth(const Light& light, const Camera& camera, uint32_t cascadeIndex) const;
        const glm::mat4& shadowsCascadeTransform(const Light& light, const Camera& camera, uint32_t cascadeIndex) const;
        /// @}

        /**
         * @name Descriptors
         */
        /// @{
        const vulkan::DescriptorHolder& lightsDescriptorHolder() const { return m_lightsDescriptorHolder; }
        const vulkan::DescriptorHolder& shadowsDescriptorHolder() const { return m_shadowsDescriptorHolder; }
        const vulkan::DescriptorHolder& materialDescriptorHolder() const { return m_materialDescriptorHolder; }
        const vulkan::DescriptorHolder& environmentDescriptorHolder() const { return m_environmentDescriptorHolder; }
        /// @}

        /**
         * @name Environment
         */
        /// @{
        const Environment& environment() const { return m_environment; }
        /// @}

        /// ----- Fore
        void foreAdd(Light& light);
        void foreAdd(Camera& camera);
        void foreAdd(Material& material);
        void foreAdd(Texture& /* texture */) {}
        void foreAdd(Mesh& /* mesh  */) {}
        void foreRemove(const Light& light);
        void foreRemove(const Camera& camera);
        void foreRemove(const Material& material);
        void foreRemove(const Texture& texture);
        void foreRemove(const Mesh& mesh);
        void foreEnvironmentTexture(const Texture* texture) { m_environment.set(texture); }

    protected:
        void initStages();
        void initResources();
        void updateStages(const Camera& camera);
        /// :ShadowsLightCameraPair
        void updateLightBundleFromCameras(const Light& light);

    protected:
        /// This bundle is for lights, allowing us to create shadow map.
        struct LightBundle {
            uint16_t id = -1;
            std::unique_ptr<ShadowsStage> shadowsStage;

            // :ShadowsLightCameraPair There will be one thread per combinaison of light/camera.
            std::unordered_map<const Camera*, Shadows> shadows;
            std::unordered_map<const Camera*, vulkan::CommandBufferThread> shadowsThreads;
        };

        struct CameraBundle {
            uint16_t id = -1;
            std::unique_ptr<IRendererStage> rendererStage;
            std::unique_ptr<vulkan::CommandBufferThread> rendererThread;

            // When different of -1u, specifies which shadows to use.
            // @note This is used by VR so that the left and right shares the same shadow maps.
            const Camera* shadowsFallbackCamera = nullptr;
        };

    private:
        Scene& m_fore;
        RenderEngine& m_engine;
        uint32_t m_frameId = 0u;
        bool m_initialized = false;

        // ----- Record
        std::vector<vk::CommandBuffer> m_commandBuffers;

        // ----- Descriptors
        vulkan::DescriptorHolder m_lightsDescriptorHolder;
        vulkan::DescriptorHolder m_shadowsDescriptorHolder;
        vulkan::DescriptorHolder m_materialDescriptorHolder;
        vulkan::DescriptorHolder m_environmentDescriptorHolder;

        // ----- Environment
        Environment m_environment;

        // ----- Resources
        std::unordered_map<const Camera*, CameraBundle> m_cameraBundles;
        std::unordered_map<const Light*, LightBundle> m_lightBundles;
        std::vector<const Mesh*> m_pendingRemovedMeshes;
    };
}
