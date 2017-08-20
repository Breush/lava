#pragma once

#include <lava/magma/materials/rm-material.hpp>

#include "./i-material-impl.hpp"

#include <lava/magma/render-scenes/render-scene.hpp>

#include "../holders/image-holder.hpp"
#include "../holders/ubo-holder.hpp"

namespace lava::magma {
    /**
     * Vulkan-based implementation of lava::RmMaterial.
     */
    class RmMaterial::Impl : public IMaterial::Impl {
    public:
        Impl(RenderScene& scene);
        ~Impl();

        // IMaterial::Impl
        void init() override final;
        void render(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                    uint32_t descriptorSetIndex) override final;

        // RmMaterial
        inline float roughness() const { return m_roughnessFactor; }
        void roughness(float factor);

        inline float metallic() const { return m_metallicFactor; }
        void metallic(float factor);

        void normal(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);
        void baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);
        void metallicRoughnessColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);

    protected:
        void updateBindings();

    public:
        struct Attribute {
            enum class Type {
                NONE,
                TEXTURE,
            };

            struct Texture {
                uint8_t* pixels;
                uint32_t width;
                uint32_t height;
                uint8_t channels;
            };

            Type type;
            union {
                Texture texture;
            };
        };

    private:
        // References
        RenderScene::Impl& m_scene;
        bool m_initialized = false;

        // Descriptor
        vk::DescriptorSet m_descriptorSet;
        vulkan::UboHolder m_uboHolder;

        // @todo Should be in the texture itself, and probably unique_ptr, so that they can be deleted
        vulkan::ImageHolder m_normalImageHolder;
        vulkan::ImageHolder m_albedoImageHolder;
        vulkan::ImageHolder m_ormImageHolder;

        // Data
        float m_roughnessFactor = 1.f;
        float m_metallicFactor = 1.f;
        Attribute m_normal;
        Attribute m_albedo;
        Attribute m_orm;
    };
}
