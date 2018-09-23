#pragma once

namespace lava::sill {
    class GameEngine;
    class Texture;
}

namespace lava::sill {
    /**
     * Handles dynamic fonts, inserting only used glyphs.
     */
    class Font {
    public:
        struct GlyphInfo {
            glm::vec2 minUv;
            glm::vec2 maxUv;
            float advance = 0.f;
        };

    public:
        Font(GameEngine& engine, const std::string& fontPath);

        std::vector<GlyphInfo> glyphsInfos(std::wstring_view u16Text);

        // Getters
        Texture& texture();
        float glyphsRatio() const { return m_glyphsRatio; }

    protected:
        // Internal
        GlyphInfo packGlyph(wchar_t c);

    private:
        // References
        GameEngine& m_engine;

        // Storage
        Texture* m_texture = nullptr;  //< Font texture (containing drawn glyphs).
        std::vector<uint8_t> m_pixels; //< Font texture pixels.
        std::unordered_map<wchar_t, GlyphInfo> m_glyphsInfos;
        uint32_t m_textureWidth;
        uint32_t m_textureHeight;

        // Low-level font info.
        std::vector<uint8_t> m_buffer; //< Font file buffer.
        stbtt_fontinfo m_stbFont;      //< Internal STB font info.
        uint32_t m_glyphMaxWidth;
        uint32_t m_glyphMaxHeight;
        float m_glyphsRatio; //< glyphMaxWidth / glyphMaxHeight.
        float m_glyphsScale;
        int m_glyphsAscent;
        const uint32_t m_maxRenderedGlyphsCount = 256u;
        uint32_t m_renderedGlyphsCount = 0u;
    };
}
