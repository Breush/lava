#include <lava/magma/material.hpp>

#include "./vulkan/texture-impl.hpp"

using namespace lava::magma;

$pimpl_class(Texture, RenderScene&, scene);

// Loaders
$pimpl_method(Texture, void, loadFromFile, const std::string&, imagePath);
$pimpl_method(Texture, void, loadFromMemory, const uint8_t*, pixels, uint32_t, width, uint32_t, height, uint8_t, channels);
