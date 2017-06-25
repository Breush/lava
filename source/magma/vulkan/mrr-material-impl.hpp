#pragma once

#include <lava/magma/mrr-material.hpp>

#include <lava/magma/render-engine.hpp>

#include "./capsule.hpp"
#include "./device.hpp"

namespace lava {
    /**
     * Vulkan-based implementation of lava::MrrMaterial.
     */
    class MrrMaterial::Impl {
    public:
        Impl(RenderEngine& engine);
        ~Impl();

        // Main interface
        void baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);
        void metallicRoughnessColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);

        // Internal interface
        void addCommands(VkCommandBuffer commandBuffer);

    protected:
        void init();
        void updateAttributesUbo();

    public:
        struct UniformBufferObject {
            bool hasBaseColorSampler = false;
            bool hasMetallicRoughnessSampler = true;
        };

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

        // UBO for attributes
        vulkan::Capsule<VkBuffer> m_uniformStagingBuffer;
        vulkan::Capsule<VkDeviceMemory> m_uniformStagingBufferMemory;
        vulkan::Capsule<VkBuffer> m_uniformBuffer;
        vulkan::Capsule<VkDeviceMemory> m_uniformBufferMemory;

        // @todo Should be in the texture itself, and probably unique_ptr, so that they can be deleted
        vulkan::Capsule<VkImage> m_baseColorImage;
        vulkan::Capsule<VkDeviceMemory> m_baseColorImageMemory;
        vulkan::Capsule<VkImageView> m_baseColorImageView;

        vulkan::Capsule<VkImage> m_metallicRoughnessImage;
        vulkan::Capsule<VkDeviceMemory> m_metallicRoughnessImageMemory;
        vulkan::Capsule<VkImageView> m_metallicRoughnessImageView;

        // Data
        Attribute m_baseColor;
        Attribute m_metallicRoughness;
    };
}
