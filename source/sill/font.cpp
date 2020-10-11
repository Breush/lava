#include <lava/sill/font.hpp>

#include <lava/sill/game-engine.hpp>

using namespace lava;
using namespace lava::sill;
using namespace lava::chamber;

Font::Font(GameEngine& engine, const std::vector<std::string>& paths, uint32_t size)
    : m_engine(engine)
{
    for (const auto& path : paths) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            logger.warning("sill.font") << "Unable to find file " << path << "." << std::endl;
            continue;
        }

        auto& fontInfo = *m_fontInfos.emplace_back(std::make_unique<FontInfo>());
        fontInfo.path = path;
        fontInfo.buffer.insert(fontInfo.buffer.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        stbtt_InitFont(&fontInfo.stbFont, fontInfo.buffer.data(), 0);

        fontInfo.glyphsScale = stbtt_ScaleForPixelHeight(&fontInfo.stbFont, size);
        stbtt_GetFontVMetrics(&fontInfo.stbFont, &fontInfo.glyphsAscent, 0, 0);
        fontInfo.glyphsAscent = fontInfo.glyphsAscent * fontInfo.glyphsScale;

        int x0, y0, x1, y1;
        stbtt_GetFontBoundingBox(&fontInfo.stbFont, &x0, &y0, &x1, &y1);
        fontInfo.glyphMaxWidth = std::ceil((x1 - x0) * fontInfo.glyphsScale);
        fontInfo.glyphMaxHeight = std::ceil((y1 - y0) * fontInfo.glyphsScale);

        // @note Texture size can't be updated dynamically,
        // because meshes could reference wrong UVs.
        m_textureWidth = std::max(m_textureWidth, fontInfo.glyphMaxWidth * m_maxRenderedGlyphsCount);
        m_textureHeight = std::max(m_textureHeight, fontInfo.glyphMaxHeight);
    }

    if (m_textureWidth == 0u || m_textureHeight == 0u) return;
    m_texture = m_engine.renderEngine().makeTexture();
    m_pixels.resize(m_textureWidth * m_textureHeight);
}

std::vector<Font::GlyphInfo> Font::glyphsInfos(u8string_view u8Text, bool skipBlank)
{
    std::vector<GlyphInfo> glyphsInfos;

    bool textureChanged = false;
    float advance = 0.f;

    uint8_t bytesLength;
    auto u = reinterpret_cast<const uint8_t*>(u8Text.data());
    auto c = utf8Codepoint(u, bytesLength);
    u += bytesLength;

    uint32_t i = bytesLength;
    uint32_t totalBytes = u8Text.size();
    while (i <= totalBytes) {
        auto nextC = utf8Codepoint(u, bytesLength);
        u += bytesLength;
        i += bytesLength;

        // Add the glyph to the texture if it does not exist yet
        auto pGlyphInfo = m_glyphsInfos.find(c);
        if (pGlyphInfo == m_glyphsInfos.end()) {
            m_glyphsInfos[c] = packGlyph(c);
            pGlyphInfo = m_glyphsInfos.find(c);

            textureChanged = textureChanged || !pGlyphInfo->second.blank;
        }

        // Push non-blank glyphInfos to the list
        if (!skipBlank || !pGlyphInfo->second.blank) {
            auto glyphInfo = pGlyphInfo->second;
            glyphInfo.xOffset += advance;
            glyphsInfos.emplace_back(glyphInfo);
            advance = 0.f;
        }

        const auto& fontInfo = *pGlyphInfo->second.fontInfo;
        auto kernAdvance = stbtt_GetCodepointKernAdvance(&fontInfo.stbFont, c, nextC) * fontInfo.glyphsScale / float(fontInfo.glyphMaxHeight);
        advance += pGlyphInfo->second.advance + kernAdvance;
        c = nextC;
    }

    if (textureChanged) {
        m_texture->loadFromMemory(m_pixels.data(), m_textureWidth, m_textureHeight, 1u);
    }

    return glyphsInfos;
}

Font::GlyphInfo Font::glyphInfoAtByte(u8string_view u8Text, uint32_t byteIndex)
{
    GlyphInfo glyphInfo;
    bool textureChanged = false;

    uint8_t bytesLength;
    auto u = reinterpret_cast<const uint8_t*>(u8Text.data());
    auto c = utf8Codepoint(u, bytesLength);
    u += bytesLength;

    uint32_t i = bytesLength;
    uint32_t totalBytes = u8Text.size();
    while (i <= totalBytes) {
        bool interestingByte = (i > byteIndex);
        auto nextC = utf8Codepoint(u, bytesLength);
        u += bytesLength;
        i += bytesLength;

        // Add the glyph to the texture if it does not exist yet
        auto pGlyphInfo = m_glyphsInfos.find(c);
        if (pGlyphInfo == m_glyphsInfos.end()) {
            m_glyphsInfos[c] = packGlyph(c);
            pGlyphInfo = m_glyphsInfos.find(c);
            textureChanged = textureChanged || !pGlyphInfo->second.blank;
        }

        glyphInfo.xOffset += pGlyphInfo->second.xOffset;

        // We are now at the interesting byte
        if (interestingByte) {
            auto xOffset = glyphInfo.xOffset;
            glyphInfo = pGlyphInfo->second;
            glyphInfo.xOffset = xOffset;
            break;
        }

        const auto& fontInfo = *pGlyphInfo->second.fontInfo;
        auto kernAdvance = stbtt_GetCodepointKernAdvance(&fontInfo.stbFont, c, nextC) * fontInfo.glyphsScale / float(fontInfo.glyphMaxHeight);
        glyphInfo.xOffset += pGlyphInfo->second.advance + kernAdvance;
        c = nextC;
    }

    if (textureChanged) {
        m_texture->loadFromMemory(m_pixels.data(), m_textureWidth, m_textureHeight, 1u);
    }

    return glyphInfo;
}

//----- Internal

Font::GlyphInfo Font::packGlyph(uint32_t c)
{
    GlyphInfo glyphInfo;

    for (auto& fontInfo : m_fontInfos) {
        glyphInfo.index = stbtt_FindGlyphIndex(&fontInfo->stbFont, c);
        glyphInfo.fontInfo = fontInfo.get();

        // Glyph does not exist in this font
        if (glyphInfo.index == 0) {
            continue;
        }

        // Basic glyph advance
        int advance, lsb;
        int x0, y0, x1, y1;
        stbtt_GetGlyphHMetrics(&fontInfo->stbFont, glyphInfo.index, &advance, &lsb);
        stbtt_GetGlyphBitmapBox(&fontInfo->stbFont, glyphInfo.index, fontInfo->glyphsScale, fontInfo->glyphsScale, &x0, &y0, &x1, &y1);

        glyphInfo.advance = (advance * fontInfo->glyphsScale - x0) / float(fontInfo->glyphMaxHeight);

        // Glyph is blank
        int width = x1 - x0;
        int height = y1 - y0;
        if (width == 0 || height == 0) {
            break;
        }

        glyphInfo.blank = false;
        glyphInfo.width = width / float(fontInfo->glyphMaxHeight);
        glyphInfo.height = height / float(fontInfo->glyphMaxHeight);
        glyphInfo.xOffset = x0 / float(fontInfo->glyphMaxHeight);
        glyphInfo.yOffset = (fontInfo->glyphsAscent + y0) / float(fontInfo->glyphMaxHeight);

        // Draw the bitmap to the texture
        auto glyphStartPosition = m_nextGlyphStartPosition;
        stbtt_MakeGlyphBitmap(&fontInfo->stbFont, m_pixels.data() + glyphStartPosition,
                                width, height, m_textureWidth, fontInfo->glyphsScale, fontInfo->glyphsScale, glyphInfo.index);
        m_nextGlyphStartPosition += width + 1;

        // Stores the uv informations.
        glyphInfo.minUv.x = static_cast<float>(glyphStartPosition) / static_cast<float>(m_textureWidth);
        glyphInfo.minUv.y = 0.f;
        glyphInfo.maxUv.x = static_cast<float>(glyphStartPosition + width) / static_cast<float>(m_textureWidth);
        glyphInfo.maxUv.y = static_cast<float>(height) / static_cast<float>(m_textureHeight);
        break;
    }

    return glyphInfo;
}
