#pragma once

#include "../vulkan/holders/image-holder.hpp"

namespace lava::magma {
    class Texture;
    class Scene;
}

namespace lava::magma {
    class TextureAft {
    public:
        TextureAft(Texture& fore, Scene& scene);

        vk::ImageView imageView() const { return m_imageHolder.view(); }

        // ----- Fore
        void foreLoadFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels);
        void foreLoadCubeFromMemory(const std::array<const uint8_t*, 6u>& imagesPixels, uint32_t width, uint32_t height);

    private:
        Texture& m_fore;
        Scene& m_scene;

        vulkan::ImageHolder m_imageHolder;
    };
}
