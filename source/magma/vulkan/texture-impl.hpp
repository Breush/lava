#pragma once

#include <lava/magma/texture.hpp>

#include <lava/magma/render-scenes/render-scene.hpp>

#include "./holders/image-holder.hpp"

namespace lava::magma {
    /**
     * Vulkan-based implementation of magma::Texture.
     */
    class Texture::Impl {
    public:
        Impl(RenderScene& scene);

        // Texture
        void loadFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels);
        void loadFromFile(const std::string& imagePath);

        // Getters
        vk::ImageView imageView() const { return m_imageHolder.view(); }

        // Internal interface
        void init();

    private:
        // References
        RenderScene::Impl& m_scene;
        bool m_initialized = false;

        // Descriptor
        vulkan::ImageHolder m_imageHolder;
    };
}
