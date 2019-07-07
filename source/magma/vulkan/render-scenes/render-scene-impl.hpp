#pragma once

#include <lava/magma/cameras/i-camera.hpp>
#include <lava/magma/lights/i-light.hpp>
#include <lava/magma/material.hpp>
#include <lava/magma/mesh.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>
#include <lava/magma/texture.hpp>

#include "../command-buffer-thread.hpp"
#include "../environment.hpp"
#include "../holders/descriptor-holder.hpp"
#include "../shadows.hpp"

namespace lava::magma {
    class IRendererStage;
    class ShadowsStage;
}

namespace lava::magma {
    /**
     * Vulkan-based implementation of the lava::RenderScene.
     */
    class RenderScene::Impl {
    public:
        // @note Each frame will be renderer with a frame id being within [0 .. FRAME_IDS_COUNT],
        // this is independent from the swapchain and is incremented during each render scene update.
        // Consider using it when you update some buffer that might be used during current render,
        // which is the case with Shadows's ubo.
        static constexpr const uint32_t FRAME_IDS_COUNT = 3u;

    public:
        Impl(RenderEngine& engine, RenderScene& scene);
        ~Impl();

        /// Get the engine implementation.
        RenderEngine::Impl& engine() { return m_engine; }
        const RenderEngine::Impl& engine() const { return m_engine; }

        // Internal interface
        void init(uint32_t id);
        void update();
        void record();
        void waitRecord();
        const std::vector<vk::CommandBuffer>& commandBuffers() const { return m_commandBuffers; }

        // User control.
        void rendererType(RendererType rendererType) { m_rendererType = rendererType; }

        /**
         * @name Allocators
         */
        /// @{
        chamber::BucketAllocator& meshAllocator() { return m_meshAllocator; }
        chamber::BucketAllocator& materialAllocator() { return m_materialAllocator; }
        /// @}

        /**
         * @name Environment
         */
        /// @{
        void environmentTexture(Texture* texture) { m_environment.set(texture->impl()); }
        /// @}

        /**
         * @name Adders
         */
        /// @{
        void add(std::unique_ptr<ICamera>&& camera);
        void add(std::unique_ptr<Material>&& material);
        void add(std::unique_ptr<Texture>&& texture);
        void add(std::unique_ptr<Mesh>&& mesh);
        void add(std::unique_ptr<ILight>&& light);
        /// @}

        /**
         * @name Removers
         */
        /// @{
        void remove(const Mesh& mesh);
        void remove(const Material& material);
        void remove(const Texture& texture);
        /// @}

        /**
         * @name Getters
         */
        /// @{
        RenderScene& scene() { return m_scene; }
        const RenderScene& scene() const { return m_scene; }

        const vulkan::DescriptorHolder& lightsDescriptorHolder() const { return m_lightsDescriptorHolder; }
        const vulkan::DescriptorHolder& shadowsDescriptorHolder() const { return m_shadowsDescriptorHolder; }
        const vulkan::DescriptorHolder& materialDescriptorHolder() const { return m_materialDescriptorHolder; }
        const vulkan::DescriptorHolder& environmentDescriptorHolder() const { return m_environmentDescriptorHolder; }
        /// @}

        /**
         * @name Internal interface
         */
        /// @{
        void updateCamera(uint32_t cameraID);

        Material::Impl& fallbackMaterial() { return m_fallbackMaterial->impl(); }
        void fallbackMaterial(std::unique_ptr<Material>&& material);

        const ICamera::Impl& camera(uint32_t index) const { return m_cameraBundles[index].camera->interfaceImpl(); }
        const Material::Impl& material(uint32_t index) const { return m_materials[index]->impl(); }
        const Mesh::Impl& mesh(uint32_t index) const { return *m_meshesImpls[index]; }
        const ILight::Impl& light(uint32_t index) const { return m_lightBundles[index].light->interfaceImpl(); }
        ILight::Impl& light(uint32_t index) { return m_lightBundles[index].light->interfaceImpl(); }
        const Shadows& shadows(uint32_t lightIndex, uint32_t cameraIndex) const;
        const Environment& environment() const { return m_environment; }

        const std::vector<std::unique_ptr<Material>>& materials() const { return m_materials; }
        const std::vector<std::unique_ptr<Texture>>& textures() const { return m_textures; }
        const std::vector<Mesh::Impl*>& meshes() const { return m_meshesImpls; }

        uint32_t camerasCount() const { return m_cameraBundles.size(); }
        uint32_t lightsCount() const { return m_lightBundles.size(); }

        RenderImage cameraRenderImage(uint32_t cameraIndex) const;
        RenderImage cameraDepthRenderImage(uint32_t cameraIndex) const;
        RenderImage shadowsCascadeRenderImage(uint32_t lightIndex, uint32_t cameraIndex = 0u, uint32_t cascadeIndex = 0u) const;
        float shadowsCascadeSplitDepth(uint32_t lightIndex, uint32_t cameraIndex, uint32_t cascadeIndex) const;
        const glm::mat4& shadowsCascadeTransform(uint32_t lightIndex, uint32_t cameraIndex, uint32_t cascadeIndex) const;
        void shadowsFallbackCamera(ICamera& camera, const ICamera& fallbackCamera);

        void changeCameraRenderImageLayout(uint32_t cameraIndex, vk::ImageLayout imageLayout, vk::CommandBuffer commandBuffer);
        /// @}

    protected:
        // Internal
        void initStages();
        void initResources();
        void updateStages(uint32_t cameraId);

    protected:
        /// This bundle is for lights, allowing us to create shadow map.
        struct LightBundle {
            std::unique_ptr<ILight> light;
            std::unique_ptr<ShadowsStage> shadowsStage;
            std::vector<std::unique_ptr<Shadows>> shadows;

            // :ShadowsLightCameraPair There will be one thread per combinaison of light/camera.
            std::vector<std::unique_ptr<vulkan::CommandBufferThread>> shadowsThreads;
        };

        struct CameraBundle {
            std::unique_ptr<ICamera> camera;
            std::unique_ptr<IRendererStage> rendererStage;
            std::unique_ptr<vulkan::CommandBufferThread> rendererThread;
            // When different of -1u, specifies which shadows to use.
            // @note This is used by VR so that the left and right shares the same shadow maps.
            uint32_t shadowsFallbackCameraId = -1u;
        };

    private:
        RenderScene& m_scene;
        RenderEngine::Impl& m_engine;
        bool m_initialized = false;
        uint32_t m_id = -1u;
        uint32_t m_frameId = 0u;

        RendererType m_rendererType;

        // Allocators
        chamber::BucketAllocator m_meshAllocator;
        chamber::BucketAllocator m_materialAllocator;

        // Resources
        vulkan::DescriptorHolder m_lightsDescriptorHolder;
        vulkan::DescriptorHolder m_shadowsDescriptorHolder;
        vulkan::DescriptorHolder m_materialDescriptorHolder;
        vulkan::DescriptorHolder m_environmentDescriptorHolder;

        // @note To be kept after m_materialDescriptorHolder,
        // to prevent delete order issues.
        std::unique_ptr<Material> m_fallbackMaterial;
        Environment m_environment;

        // Data
        std::vector<CameraBundle> m_cameraBundles;
        std::vector<LightBundle> m_lightBundles;
        std::vector<std::unique_ptr<Material>> m_materials;
        std::vector<std::unique_ptr<Texture>> m_textures;
        std::vector<std::unique_ptr<Mesh>> m_meshes;

        std::vector<const Mesh*> m_pendingRemovedMeshes;

        // Data references
        std::vector<Mesh::Impl*> m_meshesImpls;

        std::vector<vk::CommandBuffer> m_commandBuffers; // All recorded command buffers during last record() call.
    };
}
