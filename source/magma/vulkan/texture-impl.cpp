#include "./texture-impl.hpp"

#include <fstream>
#include <lava/chamber/logger.hpp>
#include <stb/stb_image.h>

#include "./render-scenes/render-scene-impl.hpp"

using namespace lava::chamber;
using namespace lava::magma;

Texture::Impl::Impl(RenderScene& scene)
    : m_scene(scene.impl())
    , m_imageHolder(m_scene.engine())
{
}

//----- Internals

void Texture::Impl::init()
{
}

//----- Texture

void Texture::Impl::loadFromMemory(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    m_imageHolder.setup(pixels, width, height, channels);
}

void Texture::Impl::loadFromFile(const std::string& imagePath)
{
    std::ifstream file(imagePath, std::ios::binary);

    if (!file.is_open()) {
        logger.warning("magma.vulkan.texture") << "Unable to find file " << imagePath << "." << std::endl;
        return;
    }

    std::vector<uint8_t> buffer;
    buffer.insert(buffer.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    int32_t width, height;
    auto pixels = stbi_load_from_memory(buffer.data(), buffer.size(), &width, &height, nullptr, 4);
    std::vector<uint8_t> pixelsVector(width * height * 4);
    memmove(pixelsVector.data(), pixels, pixelsVector.size());

    loadFromMemory(pixelsVector, width, height, 4u);

    stbi_image_free(pixels);
}
