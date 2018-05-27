#include <lava/sill/components/mesh-component.hpp>

#include <lava/core/macros.hpp>

#include "./mesh-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(MeshComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(MeshComponent, void, update);

$pimpl_method(MeshComponent, void, verticesCount, const uint32_t, count);
$pimpl_method(MeshComponent, void, verticesPositions, const std::vector<glm::vec3>&, positions);
$pimpl_method(MeshComponent, void, verticesUvs, const std::vector<glm::vec2>&, uvs);
$pimpl_method(MeshComponent, void, verticesNormals, const std::vector<glm::vec3>&, normals);
$pimpl_method(MeshComponent, void, verticesTangents, const std::vector<glm::vec4>&, tangents);
$pimpl_method(MeshComponent, void, indices, const std::vector<uint16_t>&, indices);

$pimpl_method(MeshComponent, void, material, Material&, material);
