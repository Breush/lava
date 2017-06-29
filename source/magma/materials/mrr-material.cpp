#include <lava/magma/materials/mrr-material.hpp>

#include <lava/chamber/pimpl.hpp>

#include "../vulkan/materials/mrr-material-impl.hpp"

using namespace lava;

$pimpl_class(MrrMaterial, RenderEngine&, engine);

$pimpl_method(MrrMaterial, void, normal, const std::vector<uint8_t>&, pixels, uint32_t, width, uint32_t, height, uint8_t,
              channels);
$pimpl_method(MrrMaterial, void, baseColor, const std::vector<uint8_t>&, pixels, uint32_t, width, uint32_t, height, uint8_t,
              channels);
$pimpl_method(MrrMaterial, void, metallicRoughnessColor, const std::vector<uint8_t>&, pixels, uint32_t, width, uint32_t, height,
              uint8_t, channels);
