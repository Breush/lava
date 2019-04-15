#include <lava/sill/mesh-primitive.hpp>

#include "./mesh-primitive-impl.hpp"

using namespace lava::sill;

$pimpl_class(MeshPrimitive, GameEngine&, engine);

MeshPrimitive::MeshPrimitive(MeshPrimitive&& meshPrimitive)
{
    m_impl = meshPrimitive.m_impl;
    meshPrimitive.m_impl = nullptr;
}

// Geometry
$pimpl_method(MeshPrimitive, void, verticesCount, const uint32_t, count);
$pimpl_method(MeshPrimitive, void, verticesPositions, VectorView<glm::vec3>, positions);
$pimpl_method(MeshPrimitive, void, verticesUvs, VectorView<glm::vec2>, uvs);
$pimpl_method(MeshPrimitive, void, verticesNormals, VectorView<glm::vec3>, normals);
$pimpl_method(MeshPrimitive, void, verticesTangents, VectorView<glm::vec4>, tangents);
$pimpl_method(MeshPrimitive, void, indices, VectorView<uint16_t>, indices);

// Material
$pimpl_method(MeshPrimitive, void, material, Material&, material);
$pimpl_method(MeshPrimitive, void, translucent, bool, translucent);
