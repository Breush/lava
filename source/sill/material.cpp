#include <lava/sill/material.hpp>

#include "game-engine-impl.hpp"

using namespace lava::sill;

Material::Material(GameEngine& engine, const std::string& hrid)
    : m_engine(engine)
{
    m_magma = &m_engine.impl().renderScene().make<magma::Material>(hrid);
}

Material::~Material()
{
    m_engine.impl().renderScene().remove(*m_magma);
    m_magma = nullptr;
}

//----- Material

void Material::set(const std::string& uniformName, float value)
{
    m_magma->set(uniformName, value);
}

void Material::set(const std::string& uniformName, const glm::vec4& value)
{
    m_magma->set(uniformName, value);
}

void Material::set(const std::string& uniformName, const Texture& texture)
{
    m_magma->set(uniformName, texture.magma());
}

const glm::vec4& Material::get_vec4(const std::string& uniformName) const
{
    return m_magma->get_vec4(uniformName);
}
