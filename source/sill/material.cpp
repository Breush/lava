#include <lava/sill/material.hpp>

#include "game-engine-impl.hpp"

using namespace lava::sill;

Material::Material(GameEngine& engine, const std::string& hrid)
    : m_engine(engine)
{
    m_magma = &m_engine.impl().scene().make<magma::Material>(hrid);
}

Material::~Material()
{
    m_engine.impl().scene().remove(*m_magma);
    m_magma = nullptr;
}

//----- Material

void Material::set(const std::string& uniformName, bool value)
{
    m_magma->set(uniformName, value);
}

void Material::set(const std::string& uniformName, uint32_t value)
{
    m_magma->set(uniformName, value);
}

void Material::set(const std::string& uniformName, float value)
{
    m_magma->set(uniformName, value);
}

void Material::set(const std::string& uniformName, const glm::vec2& value)
{
    m_magma->set(uniformName, value);
}

void Material::set(const std::string& uniformName, const glm::vec3& value)
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

void Material::set(const std::string& uniformName, const uint32_t* values, uint32_t size)
{
    m_magma->set(uniformName, values, size);
}

const glm::vec4& Material::get_vec4(const std::string& uniformName) const
{
    return m_magma->get_vec4(uniformName);
}
