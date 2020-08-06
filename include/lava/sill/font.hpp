#pragma once

#include <glm/vec2.hpp>
#include <lava/chamber/stb/truetype.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace lava::magma {
    class Texture;

    using TexturePtr = std::shared_ptr<Texture>;
}

namespace lava::sill {
    class GameEngine;
}

namespace lava::sill {
    /**
     * Handles dynamic fonts, inserting only used glyphs.
     */
    class Font {
    public:
        struct GlyphInfo {
            glm::vec2 minUv = glm::vec2(0.f);
            glm::vec2 maxUv = glm::vec2(0.f);

            // Position (relative to previous) and extent of the current glyph.
            float xOffset = 0.f;
            float yOffset = 0.f;
            float width = 0.f;
            float height = 1.f;

            // Horizontal offset to position the next glyph.
            // Already baked in next xOffset.
            float advance = 0.f;
        };

    public:
        Font(GameEngine& engine, const std::string& path, uint32_t size);

        // Extent is re-normalized , so that a character that take the full font size
        // that is provided will have a height of 1.
        // The skipWhite option keeps only renderable characters,
        // the xOffset being updated accordingly.
        std::vector<GlyphInfo> glyphsInfos(std::wstring_view u16Text, bool skipWhite = true);

        // Getters
        magma::TexturePtr texture() const { return m_texture; }

    protected:
        // Internal
        GlyphInfo packGlyph(wchar_t c);

    private:
        // References
        GameEngine& m_engine;

        // Storage
        magma::TexturePtr m_texture = nullptr; //< Font texture (containing drawn glyphs).
        std::vector<uint8_t> m_pixels;         //< Font texture pixels.
        std::unordered_map<wchar_t, GlyphInfo> m_glyphsInfos;
        uint32_t m_textureWidth = 0u;
        uint32_t m_textureHeight = 0u;
        uint32_t m_nextGlyphStartPosition = 1u; //< Next empty position within texture.

        // Low-level font info.
        std::vector<uint8_t> m_buffer; //< Font file buffer.
        stbtt_fontinfo m_stbFont;      //< Internal STB font info.
        uint32_t m_glyphMaxWidth = 0u;
        uint32_t m_glyphMaxHeight = 0u;
        float m_glyphsScale = 0.f;
        int m_glyphsAscent = 0;

        // @note This is to decide the texture size,
        // and is not really a "max" that will be checked.
        const uint32_t m_maxRenderedGlyphsCount = 32u;
    };
}
