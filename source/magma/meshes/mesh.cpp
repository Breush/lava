#include <lava/magma/meshes/mesh.hpp>

#include <lava/chamber/logger.hpp>
#include <lava/core/macros.hpp>

#include "../vulkan/meshes/mesh-impl.hpp"

using namespace lava::magma;
using namespace lava::chamber;

Mesh::Mesh(RenderScene& scene)
{
    m_impl = new Impl(scene);
}

Mesh::~Mesh()
{
    delete m_impl;
}

// IMesh
IMesh::Impl& Mesh::interfaceImpl()
{
    return *m_impl;
}

$pimpl_method_const(Mesh, const glm::mat4&, transform);
$pimpl_method(Mesh, void, transform, const glm::mat4&, transform);
$pimpl_method(Mesh, void, positionAdd, const glm::vec3&, delta);
$pimpl_method(Mesh, void, rotationAdd, const glm::vec3&, axis, float, angleDelta);

$pimpl_method(Mesh, void, verticesCount, const uint32_t, count);
$pimpl_method(Mesh, void, verticesPositions, const std::vector<glm::vec3>&, positions);
$pimpl_method(Mesh, void, verticesUvs, const std::vector<glm::vec2>&, uvs);
$pimpl_method(Mesh, void, verticesNormals, const std::vector<glm::vec3>&, normals);
$pimpl_method(Mesh, void, verticesTangents, const std::vector<glm::vec4>&, tangents);
$pimpl_method(Mesh, void, indices, const std::vector<uint16_t>&, indices);

$pimpl_method(Mesh, Material&, material);
$pimpl_method(Mesh, void, material, Material&, material);
