#include <lava/sill/makers/text-flat.hpp>

#include <lava/chamber/string-tools.hpp>
#include <lava/magma/flat.hpp>
#include <lava/sill/components/flat-component.hpp>
#include <lava/sill/entity.hpp>
#include <lava/sill/game-engine.hpp>

#include "./makers-common.hpp"

using namespace lava;
using namespace lava::sill;
using namespace lava::chamber;

std::function<FlatNode&(FlatComponent&)> makers::textFlatMaker(const std::wstring& text, const TextFlatOptions& options)
{
    TextOptions textOptions;
    textOptions.fontHrid = options.fontHrid;
    textOptions.fontSize = options.fontSize;
    textOptions.anchor = options.anchor;
    textOptions.alignment = options.alignment;
    textOptions.extentPtr = options.extentPtr;

    return [text, textOptions](FlatComponent& flatComponent) -> FlatNode& {
        auto& engine = flatComponent.entity().engine();
        auto geometry = textGeometry(engine, text, textOptions);

        auto& node = flatComponent.addNode();
        node.flatGroup = std::make_unique<FlatGroup>(flatComponent.entity().engine());
        auto& primitive = node.flatGroup->addPrimitive();
        primitive.verticesCount(geometry.positions.size());
        primitive.verticesPositions(geometry.positions);
        primitive.verticesUvs(geometry.uvs);
        primitive.indices(geometry.indices);

        auto material = flatComponent.entity().engine().scene2d().makeMaterial("font");
        material->set("fontTexture", geometry.texture);
        primitive.material(material);

        return node;
    };
}
