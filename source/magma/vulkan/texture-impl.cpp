#include "./texture-impl.hpp"

#include "./render-scenes/render-scene-impl.hpp"

using namespace lava::chamber;
using namespace lava::magma;

Texture::Impl::Impl(RenderScene& scene)
    : m_scene(scene.impl())
    , m_imageHolder(m_scene.engine(), "magma.vulkan.texture.image")
{
}

//----- Internals

void Texture::Impl::init() {}

//----- Texture

void Texture::Impl::loadFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels)
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

    loadFromMemory(pixels, width, height, 4u);

    stbi_image_free(pixels);
}

void Texture::Impl::loadCubeFromFiles(const std::string& imagesPath)
{
    std::vector<std::string> fileNames = {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"};

    bool firstTime = true;
    for (auto layer = 0u; layer < fileNames.size(); ++layer) {
        auto imagePath = imagesPath + fileNames[layer];
        std::ifstream file(imagePath, std::ios::binary);

        if (!file.is_open()) {
            logger.warning("magma.vulkan.texture") << "Unable to find file " << imagePath << "." << std::endl;
            return;
        }

        std::vector<uint8_t> buffer;
        buffer.insert(buffer.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        int32_t width, height;
        auto pixels = stbi_load_from_memory(buffer.data(), buffer.size(), &width, &height, nullptr, 4);

        if (firstTime) {
            firstTime = false;

            vk::Extent2D extent;
            extent.width = width;
            extent.height = height;
            m_imageHolder.create(vk::Format::eR8G8B8A8Unorm, extent, vk::ImageAspectFlagBits::eColor, 6u);
        }

        m_imageHolder.copy(pixels, 1u, layer);

        stbi_image_free(pixels);
    }
}
