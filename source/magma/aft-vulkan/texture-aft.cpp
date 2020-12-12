#include "./texture-aft.hpp"

#include <lava/magma/render-engine.hpp>

using namespace lava::magma;

TextureAft::TextureAft(Texture& fore, RenderEngine& engine)
    : m_fore(fore)
    , m_engine(engine)
    , m_imageHolder(engine.impl(), "texture")
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

    m_imageHolder.create(vulkan::ImageKind::Texture, vk::Format::eR8G8B8A8Unorm, extent, 6u);

    for (auto layer = 0u; layer < 6u; ++layer) {
        auto pixels = imagesPixels[layer];
        m_imageHolder.copy(pixels, 1u, layer);
    }
}
