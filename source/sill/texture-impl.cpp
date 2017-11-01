#include "./texture-impl.hpp"

#include "./game-engine-impl.hpp"

using namespace lava::sill;

Texture::Impl::Impl(GameEngine& engine)
    : m_engine(engine.impl())
{
    m_texture = &m_engine.renderScene().make<magma::Texture>();
}

Texture::Impl::~Impl()
{
    m_engine.renderScene().remove(*m_texture);
    m_texture = nullptr;
}

//----- Texture

void Texture::Impl::loadFromMemory(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    m_texture->loadFromMemory(pixels, width, height, channels);
}

void Texture::Impl::loadFromFile(const std::string& imagePath)
{
    m_texture->loadFromFile(imagePath);
}
