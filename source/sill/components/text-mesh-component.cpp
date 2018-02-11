#include <lava/sill/components/text-mesh-component.hpp>

#include <lava/chamber/macros.hpp>

#include "./text-mesh-component-impl.hpp"

using namespace lava::sill;

$pimpl_class_base(TextMeshComponent, IComponent, GameEntity&, entity);

// IComponent
$pimpl_method(TextMeshComponent, void, update);

$pimpl_method(TextMeshComponent, void, text, const std::wstring&, u16Text);
$pimpl_method(TextMeshComponent, void, font, const std::string&, hrid);

$pimpl_method(TextMeshComponent, void, horizontalAnchor, Anchor, horizontalAnchor);
$pimpl_method(TextMeshComponent, void, verticalAnchor, Anchor, verticalAnchor);
$pimpl_method(TextMeshComponent, void, alignment, Alignment, alignment);
