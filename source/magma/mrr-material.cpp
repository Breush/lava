#include <lava/magma/mrr-material.hpp>

#include <lava/chamber/pimpl.hpp>

#include "./vulkan/mrr-material-impl.hpp"

using namespace lava;

$pimpl_class(MrrMaterial);

// @todo Make pimpl
void MrrMaterial::baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint32_t channels)
{
}
