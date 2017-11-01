#include <lava/sill/material.hpp>

#include <lava/chamber/macros.hpp>

#include "./material-impl.hpp"

using namespace lava::sill;

$pimpl_class(Material, GameEngine&, engine, const std::string&, hrid);

// Uniform setters
$pimpl_method(Material, void, set, const std::string&, uniformName, float, value);
$pimpl_method(Material, void, set, const std::string&, uniformName, const glm::vec4&, value);
$pimpl_method(Material, void, set, const std::string&, uniformName, const Texture&, texture);
