#include "./text-mesh-component-impl.hpp"

#include <codecvt>
#include <lava/chamber/logger.hpp>
#include <lava/chamber/string-tools.hpp>
#include <lava/sill/components/mesh-component.hpp>
#include <locale>
#include <vector>

#include "../game-engine-impl.hpp"
#include "../game-entity-impl.hpp"

using namespace lava::chamber;
using namespace lava::sill;

TextMeshComponent::Impl::Impl(GameEntity& entity)
    : ComponentImpl(entity)
{
    entity.ensure<MeshComponent>();
}

//----- IComponent

void TextMeshComponent::Impl::update()
{
    if (!m_dirty) return;
    m_dirty = false;

    // Getting the font
    auto& font = entity().engine().font(m_fontHrid);
    const auto glyphsRatio = font.glyphsRatio();

    // Geometry
    std::vector<uint16_t> indices;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals(4u * m_text.size(), {0, 1, 0});
    std::vector<glm::vec2> uvs;

    //----- Build the geometry

    float yOffset = 0.f;
    auto glyphsCount = 0u;
    FloatExtent2d globalTextExtent;
    for (auto& text : splitAsViews(m_text, L'\n')) {
        const auto glyphsInfos = font.glyphsInfos(text);
        const auto textExtent = glyphsExtent(glyphsInfos);

        // @note These operation are valid because this is a left to right language
        globalTextExtent.width = std::max(globalTextExtent.width, textExtent.width);
        globalTextExtent.height += textExtent.height;

        // Find offsets for alignment
        float xOffset = 0.f;
        switch (m_alignment) {
        case Alignment::START: break;
        case Alignment::CENTER: xOffset -= textExtent.width / 2.f; break;
        case Alignment::END: xOffset -= textExtent.width; break;
        }

        // Fill up geometry
        for (const auto& glyphInfo : glyphsInfos) {
            indices.emplace_back(4u * glyphsCount + 0u);
            indices.emplace_back(4u * glyphsCount + 1u);
            indices.emplace_back(4u * glyphsCount + 2u);
            indices.emplace_back(4u * glyphsCount + 2u);
            indices.emplace_back(4u * glyphsCount + 3u);
            indices.emplace_back(4u * glyphsCount + 0u);

            positions.emplace_back(xOffset, yOffset, 0);
            positions.emplace_back(xOffset + glyphsRatio, yOffset, 0);
            positions.emplace_back(xOffset + glyphsRatio, yOffset + 1, 0);
            positions.emplace_back(xOffset, yOffset + 1, 0);

            uvs.emplace_back(glyphInfo.minUv.x, glyphInfo.minUv.y);
            uvs.emplace_back(glyphInfo.maxUv.x, glyphInfo.minUv.y);
            uvs.emplace_back(glyphInfo.maxUv.x, glyphInfo.maxUv.y);
            uvs.emplace_back(glyphInfo.minUv.x, glyphInfo.maxUv.y);

            xOffset += glyphInfo.advance;
            ++glyphsCount;
        }

        yOffset -= textExtent.height;
    }

    //----- Adjust for anchors

    // Anchor horizontally
    float xAnchor = 0.f;
    switch (m_alignment) {
    case Alignment::START: xAnchor -= globalTextExtent.width / 2.f; break;
    case Alignment::CENTER: break;
    case Alignment::END: xAnchor += globalTextExtent.width / 2.f; break;
    }
    switch (m_horizontalAnchor) {
    case Anchor::START: xAnchor += globalTextExtent.width / 2.f; break;
    case Anchor::CENTER: break;
    case Anchor::END: xAnchor -= globalTextExtent.width / 2.f; break;
    }

    // Anchor vertically
    float yAnchor = 0.f;
    switch (m_verticalAnchor) {
    case Anchor::START: break;
    case Anchor::CENTER: yAnchor += globalTextExtent.height / 2.f; break;
    case Anchor::END: yAnchor += globalTextExtent.height; break;
    }

    for (auto& position : positions) {
        position.x += xAnchor;
        position.y += yAnchor;
    }

    //----- Apply the geometry

    auto& engine = entity().engine().base();
    auto& material = engine.make<Material>("font");
    material.set("fontTexture", font.texture());

    auto& meshComponent = reinterpret_cast<MeshComponent&>(entity().getComponent("mesh"));
    meshComponent.verticesCount(positions.size());
    meshComponent.indices(indices);
    meshComponent.verticesPositions(positions);
    meshComponent.verticesNormals(normals);
    meshComponent.verticesUvs(uvs);
    meshComponent.material(material);
    meshComponent.translucent(true);
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

//----- Internal

lava::FloatExtent2d TextMeshComponent::Impl::glyphsExtent(const std::vector<Font::GlyphInfo>& glyphsInfos)
{
    FloatExtent2d extent;
    if (glyphsInfos.empty()) {
        return extent;
    }

    extent.height = 1.f;
    for (const auto& glyphInfo : glyphsInfos) {
        extent.width += glyphInfo.advance;
    }
    return extent;
}
