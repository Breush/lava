#include "./makers-common.hpp"

#include <lava/sill/game-engine.hpp>

using namespace lava;
using namespace lava::chamber;
using namespace lava::sill;

void sill::addCirclePoints(std::vector<glm::vec3>& points, std::vector<glm::vec2>& uvs, const uint32_t tessellation,
                           const float sphereRadius, const float radius, const float height)
{
    const auto uvStep = 1.f / tessellation;
    const auto step = math::TWO_PI / tessellation;
    const auto cStep = math::cos(step);
    const auto sStep = math::sin(step);

    glm::vec3 point{radius, 0.f, height};
    glm::vec2 uv{0.f, -height / sphereRadius * 0.5f + 0.5f};
    for (auto j = 0u; j < tessellation; ++j) {
        points.emplace_back(point);
        const auto px = point.x;
        point.x = px * cStep - point.y * sStep;
        point.y = px * sStep + point.y * cStep;

        uvs.emplace_back(uv);
        uv.x += uvStep;
    }

    // Last point is the first one with custom uvs
    points.emplace_back(radius, 0.f, height);
    uvs.emplace_back(1.f, uv.y);
}

void sill::addPoleStrip(std::vector<uint16_t>& indices, uint32_t tessellation, uint16_t startIndex, uint16_t poleIndex,
                        bool reverse)
{
    for (auto i = 0u; i < tessellation; i++) {
        uint16_t d0 = startIndex + i;
        uint16_t d1 = d0 + 1u;
        if (reverse) std::swap(d0, d1);
        indices.emplace_back(d0);
        indices.emplace_back(d1);
        indices.emplace_back(poleIndex);
    }
}

void sill::addRowStrip(std::vector<uint16_t>& indices, uint32_t tessellation, uint16_t startIndex)
{
    for (auto i = 0u; i < tessellation; i++) {
        uint16_t d0 = startIndex + i;
        uint16_t d1 = d0 + 1u;
        uint16_t u0 = d0 + tessellation + 1u;
        uint16_t u1 = d1 + tessellation + 1u;
        indices.emplace_back(d0);
        indices.emplace_back(d1);
        indices.emplace_back(u1);
        indices.emplace_back(u1);
        indices.emplace_back(u0);
        indices.emplace_back(d0);
    }
}

FloatExtent2d sill::glyphsExtent(const std::vector<Font::GlyphInfo>& glyphsInfos)
{
    FloatExtent2d extent;
    if (glyphsInfos.empty()) {
        return extent;
    }

    extent.height = 1.f;
    for (const auto& glyphInfo : glyphsInfos) {
        extent.width += glyphInfo.xOffset;
    }
    extent.width += glyphsInfos.back().width;
    return extent;
}

TextGeometry sill::textGeometry(GameEngine& engine, const std::wstring& text, TextOptions options)
{
    TextGeometry geometry;
    auto& positions = geometry.positions;
    auto& uvs = geometry.uvs;
    auto& indices = geometry.indices;

    auto& font = engine.font(options.fontHrid, options.fontSize);

    float yOffset = 0.f;
    auto glyphsCount = 0u;
    FloatExtent2d globalTextExtent;
    for (auto& text : splitAsViews(text, '\n')) {
        const auto glyphsInfos = font.glyphsInfos(text);
        const auto textExtent = glyphsExtent(glyphsInfos);

        // @note These operation are valid because this is a left to right language
        globalTextExtent.width = std::max(globalTextExtent.width, textExtent.width);
        globalTextExtent.height += textExtent.height;

        // Find offsets for alignment
        float xOffset = 0.f;
        switch (options.alignment) {
        case Alignment::Start: break;
        case Alignment::Center: xOffset -= textExtent.width / 2.f; break;
        case Alignment::End: xOffset -= textExtent.width; break;
        }

        // Fill up geometry
        for (const auto& glyphInfo : glyphsInfos) {
            indices.emplace_back(4u * glyphsCount + 3u);
            indices.emplace_back(4u * glyphsCount + 2u);
            indices.emplace_back(4u * glyphsCount + 0u);
            indices.emplace_back(4u * glyphsCount + 0u);
            indices.emplace_back(4u * glyphsCount + 1u);
            indices.emplace_back(4u * glyphsCount + 3u);

            xOffset += glyphInfo.xOffset;
            auto y = yOffset + glyphInfo.yOffset;
            positions.emplace_back(xOffset, y);
            positions.emplace_back(xOffset, y + glyphInfo.height);
            positions.emplace_back(xOffset + glyphInfo.width, y);
            positions.emplace_back(xOffset + glyphInfo.width, y + glyphInfo.height);

            uvs.emplace_back(glyphInfo.minUv.x, glyphInfo.minUv.y);
            uvs.emplace_back(glyphInfo.minUv.x, glyphInfo.maxUv.y);
            uvs.emplace_back(glyphInfo.maxUv.x, glyphInfo.minUv.y);
            uvs.emplace_back(glyphInfo.maxUv.x, glyphInfo.maxUv.y);

            glyphsCount += 1u;
        }

        yOffset += textExtent.height;
    }

    //----- Adjust for anchors

    // Anchor horizontally
    float xAnchor = 0.f;
    switch (options.alignment) {
    case Alignment::Start: xAnchor -= globalTextExtent.width / 2.f; break;
    case Alignment::Center: break;
    case Alignment::End: xAnchor += globalTextExtent.width / 2.f; break;
    }
    switch (options.horizontalAnchor) {
    case Anchor::Start: xAnchor += globalTextExtent.width / 2.f; break;
    case Anchor::Center: break;
    case Anchor::End: xAnchor -= globalTextExtent.width / 2.f; break;
    }

    // Anchor vertically
    float yAnchor = 0.f;
    switch (options.verticalAnchor) {
    case Anchor::Start: break;
    case Anchor::Center: yAnchor -= globalTextExtent.height / 2.f; break;
    case Anchor::End: yAnchor += globalTextExtent.height; break;
    }

    for (auto& position : positions) {
        position.x += xAnchor;
        position.y += yAnchor;
    }

    //----- Adjust for font size

    for (auto& position : positions) {
        position.x *= options.fontSize;
        position.y *= options.fontSize;
    }

    return geometry;
}
