#include "./text-mesh-component-impl.hpp"

#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>

#include "../makers/makers-common.hpp"

using namespace lava::chamber;
using namespace lava::sill;

TextMeshComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
{
    // @todo We should be able to provide a scene,
    // but this won't matter if it is :Refactor ed.
    m_entity.ensure<MeshComponent>();
}

//----- IComponent

// @todo :Refactor This should be a mesh maker (like the flat one).

void TextMeshComponent::Impl::update(float /* dt */)
{
    if (!m_dirty) return;
    m_dirty = false;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    TextOptions textOptions;
    textOptions.fontHrid = m_fontHrid;
    textOptions.fontSize = 32u; // @todo Be parametrable
    textOptions.horizontalAnchor = m_horizontalAnchor;
    textOptions.verticalAnchor = m_verticalAnchor;
    textOptions.alignment = m_alignment;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals(4u * m_text.size(), {0, 1, 0});
    auto geometry = textGeometry(m_entity.engine(), m_text, textOptions);

    // Converting vec2 positions to vec3
    positions.reserve(geometry.positions.size());
    for (const auto& position : geometry.positions) {
        positions.emplace_back(position.x, 0.f, -position.y);
    }

    auto meshGroup = std::make_unique<MeshGroup>(m_entity.get<MeshComponent>().scene());
    auto& primitive = meshGroup->addPrimitive();
    primitive.verticesCount(positions.size());
    primitive.verticesPositions(positions);
    primitive.verticesNormals(normals);
    primitive.verticesUvs(geometry.uvs);
    primitive.indices(geometry.indices);
    primitive.category(RenderCategory::Translucent);

    auto material = m_entity.engine().scene().makeMaterial("font");
    material->set("fontTexture", *geometry.texture);
    primitive.material(material);

    std::vector<MeshNode> nodes(1u);
    nodes[0u].meshGroup = std::move(meshGroup);
    nodes[0u].transform(glm::scale(glm::mat4(1.f), glm::vec3{0.01f}));

    auto& meshComponent = m_entity.get<MeshComponent>();
    meshComponent.nodes(std::move(nodes));
}

//----- Main interface

void TextMeshComponent::Impl::font(const std::string& hrid)
{
    m_fontHrid = hrid;
    m_dirty = true;
}

void TextMeshComponent::Impl::text(const std::wstring& u16Text)
{
    m_text = u16Text;
    m_dirty = true;
}

void TextMeshComponent::Impl::horizontalAnchor(Anchor horizontalAnchor)
{
    m_horizontalAnchor = horizontalAnchor;
    m_dirty = true;
}

void TextMeshComponent::Impl::verticalAnchor(Anchor verticalAnchor)
{
    m_verticalAnchor = verticalAnchor;
    m_dirty = true;
}

void TextMeshComponent::Impl::alignment(Alignment alignment)
{
    m_alignment = alignment;
    m_dirty = true;
}
