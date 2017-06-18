#pragma once

#include <lava/magma/mrr-material.hpp>

#include <lava/magma/render-engine.hpp>
#include <vulkan/vulkan.hpp>

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

        // Internal interface
        void addCommands(VkCommandBuffer commandBuffer);

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

        // Data
        Attribute m_baseColor;
    };
}
