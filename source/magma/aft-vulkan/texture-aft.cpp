#include "./texture-aft.hpp"

#include <lava/magma/scene.hpp>

using namespace lava::magma;

TextureAft::TextureAft(Texture& fore, Scene& scene)
    : m_fore(fore)
    , m_scene(scene)
    , m_imageHolder(scene.engine().impl(), "magma.vulkan.texture.image")
{
}

// ----- Fore

void TextureAft::foreLoadFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    m_imageHolder.setup(pixels, width, height, channels);
}

void TextureAft::foreLoadCubeFromMemory(const std::array<const uint8_t*, 6u>& imagesPixels, uint32_t width, uint32_t height)
{
    vk::Extent2D extent;
    extent.width = width;
    extent.height = height;

    bool firstTime = true;
    for (auto layer = 0u; layer < 6u; ++layer) {
        auto pixels = imagesPixels[layer];

        if (firstTime) {
            firstTime = false;
            m_imageHolder.create(vk::Format::eR8G8B8A8Unorm, extent, vk::ImageAspectFlagBits::eColor, 6u);
        }

        m_imageHolder.copy(pixels, 1u, layer);
    }
}
