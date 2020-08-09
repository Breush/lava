#include <lava/magma/texture.hpp>

#include "./aft-vulkan/texture-aft.hpp"

using namespace lava::chamber;
using namespace lava::magma;

Texture::Texture(RenderEngine& engine, const std::string& imagePath)
    : m_engine(engine)
{
    new (&aft()) TextureAft(*this, m_engine);

    if (!imagePath.empty()) {
        loadFromFile(imagePath);
    }
}

Texture::~Texture()
{
    aft().~TextureAft();
}

// ----- Loaders

size_t Texture::hash(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    size_t hash = 0u;
    std::hash<uint32_t> hasher;

    uint32_t length = width * height;
    for (uint32_t i = 0u; i < length; ++i) {
        hash ^= hasher(i + static_cast<uint32_t>(pixels[i]));
    }

    hash ^= (width << 2u) + (height << 1u) + channels;
    return hash;
}

void Texture::loadFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    m_cube = false;
    m_hash = hash(pixels, width, height, channels);

    aft().foreLoadFromMemory(pixels, width, height, channels);
}

void Texture::loadFromFile(const std::string& imagePath)
{
    std::ifstream file(imagePath, std::ios::binary);

    if (!file.is_open()) {
        logger.warning("magma.texture") << "Unable to find file " << imagePath << "." << std::endl;
        return;
    }

    if (m_name.empty()) {
        m_name = imagePath;
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
    m_cube = true;
    // @note We don't hash cubes because they are too rare.

    aft().foreLoadCubeFromMemory(imagesPixels, width, height);
}

void Texture::loadCubeFromFiles(const fs::Path& imagesPath)
{
    std::vector<std::string> fileNames = {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"};

    std::array<std::vector<uint8_t>, 6u> imagesBuffers;
    std::array<const uint8_t*, 6u> imagesPixels;

    int32_t width, height;
    for (auto i = 0u; i < 6u; ++i) {
        auto imagePath = imagesPath / fileNames[i];
        std::ifstream file(imagePath.string(), std::ios::binary);

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
