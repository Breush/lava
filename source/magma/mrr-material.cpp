#include <lava/magma/mrr-material.hpp>

#include <lava/chamber/pimpl.hpp>

#include "./vulkan/mrr-material-impl.hpp"

using namespace lava;

$pimpl_class(MrrMaterial, RenderEngine&, engine);

void MrrMaterial::baseColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    m_impl->baseColor(pixels, width, height, channels);
}

void MrrMaterial::metallicRoughnessColor(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels)
{
    m_impl->metallicRoughnessColor(pixels, width, height, channels);
}
