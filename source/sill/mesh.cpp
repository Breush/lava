#include <lava/sill/mesh.hpp>

#include "./mesh-impl.hpp"

using namespace lava::sill;

$pimpl_class(Mesh);

// User info
$pimpl_property(Mesh, std::string, name);

// Primitives
$pimpl_method(Mesh, MeshPrimitive&, primitive, uint32_t, index);
$pimpl_method_const(Mesh, const std::vector<MeshPrimitive>&, primitives);
$pimpl_method(Mesh, std::vector<MeshPrimitive>&, primitives);

void Mesh::primitives(std::vector<MeshPrimitive>&& primitives)
{
    m_impl->primitives(std::move(primitives));
}

$pimpl_method(Mesh, MeshPrimitive&, addPrimitive, GameEngine&, engine);
