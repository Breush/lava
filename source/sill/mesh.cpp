#include <lava/sill/mesh.hpp>

#include "./mesh-impl.hpp"

using namespace lava;
using namespace lava::sill;

$pimpl_class(Mesh, GameEngine&, engine);

// User info
$pimpl_property(Mesh, std::string, name);

// Primitives
$pimpl_method_const(Mesh, const magma::Mesh&, primitive, uint32_t, index);
$pimpl_method(Mesh, magma::Mesh&, primitive, uint32_t, index);

$pimpl_method_const(Mesh, const std::vector<magma::Mesh*>&, primitives);
$pimpl_method(Mesh, std::vector<magma::Mesh*>&, primitives);

$pimpl_method(Mesh, magma::Mesh&, addPrimitive);
