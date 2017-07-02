#include <lava/magma/materials/rm-material.hpp>

#include <lava/chamber/macros.hpp>

#include "../vulkan/materials/rm-material-impl.hpp"

using namespace lava::magma;

$pimpl_class(RmMaterial, RenderEngine&, engine);

$pimpl_method(RmMaterial, void, normal, const std::vector<uint8_t>&, pixels, uint32_t, width, uint32_t, height, uint8_t,
              channels);
$pimpl_method(RmMaterial, void, baseColor, const std::vector<uint8_t>&, pixels, uint32_t, width, uint32_t, height, uint8_t,
              channels);
$pimpl_method(RmMaterial, void, metallicRoughnessColor, const std::vector<uint8_t>&, pixels, uint32_t, width, uint32_t, height,
              uint8_t, channels);
