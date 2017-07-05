#pragma once

#include <lava/magma/materials/rm-material.hpp>

#include <lava/magma/render-engine.hpp>

#include "../capsule.hpp"

namespace lava::magma {
    /**
     * Vulkan-based implementation of lava::RmMaterial.
     */
    class RmMaterial::Impl {
        constexpr static const auto DESCRIPTOR_SET_INDEX = 1u;

        struct MaterialUbo {
            float roughnessFactor;
            float metallicFactor;
        };

    public:
        Impl(RenderEngine& engine);
        ~Impl();

        // IMaterial
        IMaterial::UserData render(IMaterial::UserData data);

        // RmMaterial
        inline float roughness() const { return m_roughnessFactor; }
        void roughness(float factor);

        inline float metallic() const { return m_metallicFactor; }
        void metallic(float factor);

        void normal(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);
        void baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);
        void metallicRoughnessColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);

    protected:
        void init();

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
        RenderEngine::Impl& m_engine;

        // Descriptor
        VkDescriptorSet m_descriptorSet;
        vulkan::Capsule<VkBuffer> m_uniformStagingBuffer;
        vulkan::Capsule<VkDeviceMemory> m_uniformStagingBufferMemory;
        vulkan::Capsule<VkBuffer> m_uniformBuffer;
        vulkan::Capsule<VkDeviceMemory> m_uniformBufferMemory;

        // @todo Should be in the texture itself, and probably unique_ptr, so that they can be deleted
        vulkan::Capsule<VkImage> m_normalImage;
        vulkan::Capsule<VkDeviceMemory> m_normalImageMemory;
        vulkan::Capsule<VkImageView> m_normalImageView;

        vulkan::Capsule<VkImage> m_albedoImage;
        vulkan::Capsule<VkDeviceMemory> m_albedoImageMemory;
        vulkan::Capsule<VkImageView> m_albedoImageView;

        vulkan::Capsule<VkImage> m_ormImage;
        vulkan::Capsule<VkDeviceMemory> m_ormImageMemory;
        vulkan::Capsule<VkImageView> m_ormImageView;

        // Data
        float m_roughnessFactor = 1.f;
        float m_metallicFactor = 1.f;
        Attribute m_normal;
        Attribute m_albedo;
        Attribute m_orm;
    };
}
