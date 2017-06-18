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
        Impl();
        ~Impl();

        void init(RenderEngine& engine);

        // Main interface
        void baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);

        // Internal interface
        void addCommands(VkCommandBuffer commandBuffer);

    protected:
        void rebindBaseColor();

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
        RenderEngine::Impl* m_engine = nullptr;

        // @todo Should be in the texture itself
        vulkan::Capsule<VkImage>* m_textureImage = nullptr;
        vulkan::Capsule<VkDeviceMemory>* m_textureImageMemory = nullptr;
        vulkan::Capsule<VkImageView>* m_textureImageView = nullptr;

        // Data
        Attribute m_baseColor;
    };
}
