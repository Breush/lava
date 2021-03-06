#include <lava/sill/components/text-mesh-component.hpp>

#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/entity.hpp>

#include "../makers/makers-common.hpp"

using namespace lava::chamber;
using namespace lava::sill;

TextMeshComponent::TextMeshComponent(Entity& entity)
    : IComponent(entity)
{
    // @todo We should be able to provide a scene,
    // but this won't matter if it is :Refactor ed.
    m_entity.ensure<MeshComponent>();
}

//----- IComponent

// @todo :Refactor This should be a mesh maker (like the flat one).

void TextMeshComponent::update(float /* dt */)
{
    if (!m_dirty) return;
    m_dirty = false;

    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    TextOptions textOptions;
    textOptions.fontHrid = m_fontHrid;
    textOptions.fontSize = 32u; // @todo Be parametrable
    textOptions.anchor = m_anchor;
    textOptions.alignment = m_alignment;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    auto geometry = textGeometry(m_entity.engine(), m_u8Text, textOptions);

    // Converting vec2 positions to vec3
    positions.reserve(geometry.positions.size());
    normals.resize(geometry.positions.size(), {0, 1, 0});
    for (const auto& position : geometry.positions) {
        positions.emplace_back(position.x, 0.f, -position.y);
    }

    auto& meshComponent = m_entity.get<MeshComponent>();
    meshComponent.removeNodes();

    auto nodeIndex = meshComponent.addNode();
    auto& group = meshComponent.nodeMakeGroup(nodeIndex);
    meshComponent.nodeMatrix(nodeIndex, glm::scale(glm::mat4(1.f), glm::vec3{0.01f}));

    auto& primitive = group.addPrimitive();
    primitive.verticesCount(positions.size());
    primitive.verticesPositions(positions);
    primitive.verticesNormals(normals);
    primitive.verticesUvs(geometry.uvs);
    primitive.indices(geometry.indices);
    primitive.renderCategory(RenderCategory::Translucent);

    auto material = m_entity.engine().scene().makeMaterial("font");
    material->set("fontTexture", geometry.texture);
    primitive.material(material);
}

//----- Main interface

void TextMeshComponent::text(const u8string& u8Text)
{
    m_u8Text = u8Text;
    m_dirty = true;
}

void TextMeshComponent::font(const std::string& hrid)
{
    m_fontHrid = hrid;
    m_dirty = true;
}

void TextMeshComponent::anchor(Anchor anchor)
{
    m_anchor = anchor;
    m_dirty = true;
}

void TextMeshComponent::alignment(Alignment alignment)
{
    m_alignment = alignment;
    m_dirty = true;
}
