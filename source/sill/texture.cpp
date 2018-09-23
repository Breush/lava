#include <lava/sill/texture.hpp>

#include "./game-engine-impl.hpp"

using namespace lava::sill;

Texture::Texture(GameEngine& engine)
    : m_engine(engine)
{
    m_magma = &m_engine.impl().renderScene().make<magma::Texture>();
}

Texture::Texture(GameEngine& engine, const std::string& imagePath)
    : Texture(engine)
{
    loadFromFile(imagePath);
}

Texture::~Texture()
{
    m_engine.impl().renderScene().remove(*m_magma);
    m_magma = nullptr;
}

//----- Texture

void Texture::loadFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    m_magma->loadFromMemory(pixels, width, height, channels);
}

void Texture::loadFromFile(const std::string& imagePath)
{
    m_magma->loadFromFile(imagePath);
}
