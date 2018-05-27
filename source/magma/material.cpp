#include <lava/magma/material.hpp>

#include <lava/core/macros.hpp>

#include "./vulkan/material-impl.hpp"

using namespace lava::magma;

$pimpl_class(Material, RenderScene&, scene, const std::string&, hrid);

// Uniform setters
$pimpl_method(Material, void, set, const std::string&, uniformName, float, value);
$pimpl_method(Material, void, set, const std::string&, uniformName, const glm::vec4&, value);
$pimpl_method(Material, void, set, const std::string&, uniformName, const Texture&, texture);
