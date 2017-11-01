#include "./material-impl.hpp"

#include <cstring>
#include <fstream>
#include <lava/chamber/logger.hpp>
#include <stb/stb_image.h>

#include "./game-engine-impl.hpp"
#include "./texture-impl.hpp"

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

void Material::Impl::set(const std::string& uniformName, const Texture& texture)
{
    m_material->set(uniformName, texture.impl().texture());
}
