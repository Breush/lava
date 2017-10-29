#include "./material-impl.hpp"

#include <cstring>
#include <fstream>
#include <lava/chamber/logger.hpp>
#include <stb/stb_image.h>

#include "./game-engine-impl.hpp"

using namespace lava::chamber;
using namespace lava::sill;

Material::Impl::Impl(GameEngine& engine, const std::string& hrid)
    : m_engine(engine.impl())
{
    m_material = &m_engine.renderScene().make<magma::Material>(hrid);
}

Material::Impl::~Impl()
{
    m_engine.renderScene().remove(*m_material);
    m_material = nullptr;
}

//----- Material

void Material::Impl::set(const std::string& uniformName, float value)
{
    m_material->set(uniformName, value);
}

void Material::Impl::set(const std::string& uniformName, const glm::vec4& value)
{
    m_material->set(uniformName, value);
}

void Material::Impl::set(const std::string& uniformName, const std::string& imagePath)
{
    std::ifstream file(imagePath, std::ios::binary);

    if (!file.is_open()) {
        logger.warning("sill.material") << "Unable to find file " << imagePath << "." << std::endl;
        return;
    }

    std::vector<uint8_t> buffer;
    buffer.insert(buffer.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    // @fixme Have a wrapper for image loading with C++ interface
    int32_t width, height;
    auto pixels = stbi_load_from_memory(buffer.data(), buffer.size(), &width, &height, nullptr, 4);
    std::vector<uint8_t> pixelsVector(width * height * 4);
    memmove(pixelsVector.data(), pixels, pixelsVector.size());
    m_material->set(uniformName, pixelsVector, width, height, 4u);
    stbi_image_free(pixels);
}

void Material::Impl::set(const std::string& uniformName, const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height,
                         uint8_t channels)
{
    m_material->set(uniformName, pixels, width, height, channels);
}
