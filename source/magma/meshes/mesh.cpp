#include <lava/magma/meshes/mesh.hpp>

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
$pimpl_method(Mesh, void, translate, const glm::vec3&, delta);
$pimpl_method(Mesh, void, rotate, const glm::vec3&, axis, float, angleDelta);
$pimpl_method(Mesh, void, scale, float, factor);

$pimpl_method(Mesh, void, verticesCount, const uint32_t, count);
$pimpl_method(Mesh, void, verticesPositions, VectorView<glm::vec3>, positions);
$pimpl_method(Mesh, void, verticesUvs, VectorView<glm::vec2>, uvs);
$pimpl_method(Mesh, void, verticesNormals, VectorView<glm::vec3>, normals);
$pimpl_method(Mesh, void, verticesTangents, VectorView<glm::vec4>, tangents);
$pimpl_method(Mesh, void, indices, VectorView<uint16_t>, indices);

$pimpl_method(Mesh, Material&, material);
$pimpl_method(Mesh, void, material, Material&, material);

$pimpl_method_const(Mesh, bool, canCastShadows);
$pimpl_method(Mesh, void, canCastShadows, bool, canCastShadows);
