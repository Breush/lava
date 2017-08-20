#include <lava/magma/materials/rm-material.hpp>

#include <lava/chamber/macros.hpp>

#include "../vulkan/materials/rm-material-impl.hpp"

using namespace lava::magma;

$pimpl_class(RmMaterial, RenderScene&, scene);

// IMaterial
$pimpl_method(RmMaterial, void, init);
$pimpl_method(RmMaterial, IMaterial::UserData, render, UserData, data);

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
