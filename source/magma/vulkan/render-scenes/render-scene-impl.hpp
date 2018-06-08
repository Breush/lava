#pragma once

// @todo Have every interface organized this way:
// in their own folder. And renamed as i-xxx.hpp
#include "./i-render-scene-impl.hpp"

#include <lava/core/macros.hpp>
#include <lava/magma/cameras/i-camera.hpp>
#include <lava/magma/lights/i-light.hpp>
#include <lava/magma/material.hpp>
#include <lava/magma/meshes/i-mesh.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>
#include <lava/magma/texture.hpp>

#include "../holders/descriptor-holder.hpp"

namespace lava::magma {
    class DeepDeferredStage;
}

namespace lava::magma {
    /**
     * Vulkan-based implementation of the lava::RenderScene.
     */
    class RenderScene::Impl final : public IRenderScene::Impl {
    public:
        Impl(RenderEngine& engine);
        ~Impl();

        /// Get the engine implementation.
        RenderEngine::Impl& engine() { return m_engine; }
        const RenderEngine::Impl& engine() const { return m_engine; }

        // IRenderScene
        void init(uint32_t id) override final;

        void render(vk::CommandBuffer commandBuffer) override final;
        RenderImage cameraRenderImage(uint32_t cameraIndex) const override final;
        RenderImage cameradepthRenderImage(uint32_t cameraIndex) const override final;

        /**
         * @name Adders
         */
        /// @{
        void add(std::unique_ptr<ICamera>&& camera);
        void add(std::unique_ptr<Material>&& material);
        void add(std::unique_ptr<Texture>&& texture);
        void add(std::unique_ptr<IMesh>&& mesh);
        void add(std::unique_ptr<ILight>&& light);
        /// @}

        /**
         * @name Removers
         */
        /// @{
        void remove(const IMesh& mesh);
        void remove(const Material& material);
        void remove(const Texture& texture);
        /// @}

        /**
         * @name Getters
         */
        /// @{
        const vulkan::DescriptorHolder& cameraDescriptorHolder() const { return m_cameraDescriptorHolder; }
        const vulkan::DescriptorHolder& materialDescriptorHolder() const { return m_materialDescriptorHolder; }
        const vulkan::DescriptorHolder& meshDescriptorHolder() const { return m_meshDescriptorHolder; }
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
        const IMesh::Impl& mesh(uint32_t index) const { return m_meshes[index]->interfaceImpl(); }
        const ILight::Impl& light(uint32_t index) const { return m_lights[index]->interfaceImpl(); }

        const std::vector<std::unique_ptr<Material>>& materials() const { return m_materials; }
        const std::vector<std::unique_ptr<Texture>>& textures() const { return m_textures; }
        const std::vector<std::unique_ptr<IMesh>>& meshes() const { return m_meshes; }
        const std::vector<std::unique_ptr<ILight>>& lights() const { return m_lights; }
        /// @}

    protected:
        // Internal
        void initStages();
        void initResources();
        void updateStages(uint32_t cameraId);

    protected:
        struct CameraBundle {
            std::unique_ptr<ICamera> camera;
            std::unique_ptr<DeepDeferredStage> deepDeferredStage;
        };

    private:
        RenderEngine::Impl& m_engine;
        bool m_initialized = false;
        uint32_t m_id = -1u;

        // Resources
        std::unique_ptr<Material> m_fallbackMaterial;
        vulkan::DescriptorHolder m_cameraDescriptorHolder;
        vulkan::DescriptorHolder m_materialDescriptorHolder;
        vulkan::DescriptorHolder m_meshDescriptorHolder;

        // Data
        std::vector<CameraBundle> m_cameraBundles;
        std::vector<std::unique_ptr<Material>> m_materials;
        std::vector<std::unique_ptr<Texture>> m_textures;
        std::vector<std::unique_ptr<IMesh>> m_meshes;
        std::vector<std::unique_ptr<ILight>> m_lights;
    };
}
