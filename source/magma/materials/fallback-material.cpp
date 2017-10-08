#include "./fallback-material.hpp"

#include <fstream>
#include <lava/chamber/macros.hpp>
#include <sstream>

#include "../vulkan/materials/fallback-material-impl.hpp"

std::string lava::magma::FallbackMaterial::hrid()
{
    return "fallback";
}

std::string lava::magma::FallbackMaterial::shaderImplementation()
{
    std::ifstream fileStream("./data/shaders/materials/fallback-material.simpl");
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

using namespace lava::magma;

$pimpl_class(FallbackMaterial, RenderScene&, scene);

IMaterial::Impl& FallbackMaterial::interfaceImpl()
{
    return *m_impl;
}
