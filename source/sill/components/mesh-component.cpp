#include <lava/sill/components/mesh-component.hpp>

#include "./mesh-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(MeshComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(MeshComponent, void, update, float, dt);

// MeshComponent
$pimpl_method(MeshComponent, MeshNode&, node, uint32_t, index);
$pimpl_method_const(MeshComponent, const std::vector<MeshNode>&, nodes);

void MeshComponent::nodes(std::vector<MeshNode>&& nodes)
{
    m_impl->nodes(std::move(nodes));
}
