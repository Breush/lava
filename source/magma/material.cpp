#include <lava/magma/material.hpp>

#include "./vulkan/material-impl.hpp"

#include "./vulkan/render-scenes/render-scene-impl.hpp"

using namespace lava::magma;

Material::Material(RenderScene& scene, const std::string& hrid)
    : m_scene(scene)
{
    m_impl = m_scene.impl().materialAllocator().allocate<Material::Impl>(scene, hrid);
}

Material::~Material()
{
    m_scene.impl().materialAllocator().deallocate(m_impl);
}

// Uniform setters
$pimpl_method(Material, void, set, const std::string&, uniformName, uint32_t, value);
$pimpl_method(Material, void, set, const std::string&, uniformName, float, value);
$pimpl_method(Material, void, set, const std::string&, uniformName, const glm::vec2&, value);
$pimpl_method(Material, void, set, const std::string&, uniformName, const glm::vec3&, value);
$pimpl_method(Material, void, set, const std::string&, uniformName, const glm::vec4&, value);
$pimpl_method(Material, void, set, const std::string&, uniformName, const Texture&, texture);
$pimpl_method(Material, void, set, const std::string&, uniformName, const uint32_t*, values, uint32_t, size);

// Uniform getters
$pimpl_method_const(Material, const glm::vec4&, get_vec4, const std::string&, uniformName);
