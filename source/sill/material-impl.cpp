#include "./material-impl.hpp"

#include "./game-engine-impl.hpp"

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

void Material::Impl::set(const std::string& uniformName, const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height,
                         uint8_t channels)
{
    m_material->set(uniformName, pixels, width, height, channels);
}
