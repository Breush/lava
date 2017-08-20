#pragma once

// @todo Have every interface organized this way:
// in their own folder. And renamed as i-xxx.hpp
#include "./i-render-scene-impl.hpp"

#include <lava/chamber/macros.hpp>
#include <lava/magma/interfaces/camera.hpp>
#include <lava/magma/interfaces/material.hpp>
#include <lava/magma/interfaces/mesh.hpp>
#include <lava/magma/interfaces/point-light.hpp>
#include <lava/magma/render-scenes/render-scene.hpp>

#include "../holders/descriptor-holder.hpp"
#include "../stages/epiphany.hpp"
#include "../stages/g-buffer.hpp"

namespace lava::magma {
    /**
     * Vulkan-based implementation of the lava::RenderScene.
     */
    class RenderScene::Impl final : public IRenderScene::Impl {
    public:
        Impl(RenderEngine& engine);

        /// Get the engine implementation.
        RenderEngine::Impl& engine() { return m_engine; }
        const RenderEngine::Impl& engine() const { return m_engine; }

        // IRenderScene
        void init() override final;

        Extent2d extent() const override final { return {m_extent.width, m_extent.height}; }
        void extent(Extent2d extent) override final;

        void render(vk::CommandBuffer commandBuffer) override final;
        vk::ImageView imageView() const override final { return m_epiphany.imageView(); }

        /**
         * @name Adders
         */
        /// @{
        void add(std::unique_ptr<ICamera>&& camera);
        void add(std::unique_ptr<IMaterial>&& material);
        void add(std::unique_ptr<IMesh>&& mesh);
        void add(std::unique_ptr<IPointLight>&& pointLight);
        /// @}

        /**
         * @name Getters
         */
        /// @{
        const vulkan::DescriptorHolder& cameraDescriptorHolder() const { return m_gBuffer.cameraDescriptorHolder(); }
        const vulkan::DescriptorHolder& materialDescriptorHolder() const { return m_gBuffer.materialDescriptorHolder(); }
        const vulkan::DescriptorHolder& meshDescriptorHolder() const { return m_gBuffer.meshDescriptorHolder(); }
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
        // Internal
        void initStages();
        void updateStages();
        void initResources();

    private:
        RenderEngine::Impl& m_engine;
        bool m_initialized = false;

        // IRenderScene
        vk::Extent2D m_extent;

        // Rendering stages
        GBuffer m_gBuffer{*this};
        Epiphany m_epiphany{*this};

        // Data
        std::vector<std::unique_ptr<ICamera>> m_cameras;
        std::vector<std::unique_ptr<IMaterial>> m_materials;
        std::vector<std::unique_ptr<IMesh>> m_meshes;
        std::vector<std::unique_ptr<IPointLight>> m_pointLights;
    };
}
