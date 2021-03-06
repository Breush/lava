#pragma once

#include <lava/core/u8string.hpp>
#include <lava/sill/font.hpp>

namespace lava::sill {
    void addCirclePoints(std::vector<glm::vec3>& points, std::vector<glm::vec2>& uvs, const uint32_t tessellation,
                         const float sphereRadius, const float radius, const float height);

    void addPoleStrip(std::vector<uint16_t>& indices, uint32_t tessellation, uint16_t startIndex, uint16_t poleIndex,
                      bool reverse);

    void addRowStrip(std::vector<uint16_t>& indices, uint32_t tessellation, uint16_t startIndex);

    /// Bounding extent of all glyphs list.
    FloatExtent2d glyphsExtent(const std::vector<Font::GlyphInfo>& glyphsInfos);

    struct TextGeometry {
        std::vector<uint16_t> indices;
        std::vector<glm::vec2> positions;
        std::vector<glm::vec2> uvs;
        magma::TexturePtr texture;
    };

    // @todo :Refactor Could be shared with the TextFlat maker one
    struct TextOptions {
        std::string fontHrid = "default";
        uint32_t fontSize = 32u;
        Anchor anchor = Anchor::Center;
        Alignment alignment = Alignment::Center;
        FloatExtent2d* extentPtr = nullptr;
    };

    TextGeometry textGeometry(GameEngine& engine, const u8string& u8Text, const TextOptions& options);
}
