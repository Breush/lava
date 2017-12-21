#include <lava/sill/material.hpp>

#include <lava/chamber/macros.hpp>
#include <lava/magma/material.hpp>

#include "game-engine-impl.hpp"

using namespace lava::sill;

Material::Material(GameEngine& engine, const std::string& hrid)
    : m_engine(engine)
{
    m_original = &m_engine.impl().renderScene().make<magma::Material>(hrid);
}

Material::~Material()
{
    m_engine.impl().renderScene().remove(*m_original);
    m_original = nullptr;
}

//----- Material

void Material::set(const std::string& uniformName, float value)
{
    m_original->set(uniformName, value);
}

void Material::set(const std::string& uniformName, const glm::vec4& value)
{
    m_original->set(uniformName, value);
}

void Material::set(const std::string& uniformName, const Texture& texture)
{
    m_original->set(uniformName, texture.original());
}
