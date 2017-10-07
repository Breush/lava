#include <lava/magma/materials/rm-material.hpp>

#include <fstream>
#include <lava/chamber/macros.hpp>
#include <sstream>

#include "../macros.hpp"
#include "../vulkan/materials/rm-material-impl.hpp"

$magma_material(lava::magma::RmMaterial);

std::string lava::magma::RmMaterial::shaderImplementation()
{
    std::ifstream fileStream("./data/shaders/materials/rm-material.simpl");
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

using namespace lava::magma;

$pimpl_class(RmMaterial, RenderScene&, scene);

// IMaterial
IMaterial::Impl& RmMaterial::interfaceImpl()
{
    return *m_impl;
}

$pimpl_method_const(RmMaterial, float, roughness);
$pimpl_method(RmMaterial, void, roughness, float, factor);

$pimpl_method_const(RmMaterial, float, metallic);
$pimpl_method(RmMaterial, void, metallic, float, factor);

$pimpl_method(RmMaterial, void, normal, const std::vector<uint8_t>&, pixels, uint32_t, width, uint32_t, height, uint8_t,
              channels);
$pimpl_method(RmMaterial, void, baseColor, const std::vector<uint8_t>&, pixels, uint32_t, width, uint32_t, height, uint8_t,
              channels);
$pimpl_method(RmMaterial, void, metallicRoughnessColor, const std::vector<uint8_t>&, pixels, uint32_t, width, uint32_t, height,
              uint8_t, channels);
