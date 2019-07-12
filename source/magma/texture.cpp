#include <lava/magma/texture.hpp>

#include "./aft-vulkan/texture-aft.hpp"

using namespace lava::chamber;
using namespace lava::magma;

Texture::Texture(RenderScene& scene)
    : m_scene(scene)
{
    new (&aft()) TextureAft(*this, m_scene.impl());
}

Texture::~Texture()
{
    aft().~TextureAft();
}

// ----- Loaders

void Texture::loadFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    aft().foreLoadFromMemory(pixels, width, height, channels);
}

void Texture::loadFromFile(const std::string& imagePath)
{
    std::ifstream file(imagePath, std::ios::binary);

    if (!file.is_open()) {
        logger.warning("magma.texture") << "Unable to find file " << imagePath << "." << std::endl;
        return;
    }

    std::vector<uint8_t> buffer;
    buffer.insert(buffer.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    int32_t width, height;
    auto pixels = stbi_load_from_memory(buffer.data(), buffer.size(), &width, &height, nullptr, 4);

    loadFromMemory(pixels, width, height, 4u);

    stbi_image_free(pixels);
}

void Texture::loadCubeFromMemory(const std::array<const uint8_t*, 6u>& imagesPixels, uint32_t width, uint32_t height)
{
    aft().foreLoadCubeFromMemory(imagesPixels, width, height);
}

void Texture::loadCubeFromFiles(const std::string& imagesPath)
{
    std::vector<std::string> fileNames = {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"};

    std::array<std::vector<uint8_t>, 6u> imagesBuffers;
    std::array<const uint8_t*, 6u> imagesPixels;

    int32_t width, height;
    for (auto i = 0u; i < 6u; ++i) {
        auto imagePath = imagesPath + fileNames[i];
        std::ifstream file(imagePath, std::ios::binary);

        if (!file.is_open()) {
            logger.warning("magma.texture") << "Unable to find file " << imagePath << "." << std::endl;
            return;
        }

        auto& buffer = imagesBuffers[i];
        buffer.insert(buffer.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        imagesPixels[i] = stbi_load_from_memory(buffer.data(), buffer.size(), &width, &height, nullptr, 4);
    }

    loadCubeFromMemory(imagesPixels, width, height);

    for (auto pixels : imagesPixels) {
        stbi_image_free((void*)pixels);
    }
}
