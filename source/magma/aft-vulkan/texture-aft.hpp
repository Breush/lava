#pragma once

#include "../vulkan/holders/image-holder.hpp"

namespace lava::magma {
    class Texture;
    class RenderEngine;
}

namespace lava::magma {
    class TextureAft {
    public:
        TextureAft(Texture& fore, RenderEngine& engine);

        vk::ImageView imageView() const { return m_imageHolder.view(); }
        const vulkan::ImageHolder& imageHolder() const { return m_imageHolder; }

        // ----- Fore
        void foreLoadFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels);
        void foreLoadCubeFromMemory(const std::array<const uint8_t*, 6u>& imagesPixels, uint32_t width, uint32_t height);

    private:
        Texture& m_fore;
        RenderEngine& m_engine;

        vulkan::ImageHolder m_imageHolder;
    };
}
