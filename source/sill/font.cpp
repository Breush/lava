#include <lava/sill/font.hpp>

#include <lava/sill/game-engine.hpp>

using namespace lava;
using namespace lava::sill;
using namespace lava::chamber;

Font::Font(GameEngine& engine, const std::string& path, uint32_t size)
    : m_engine(engine)
{
    //----- Read file

    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        logger.warning("sill.font") << "Unable to find file " << path << "." << std::endl;
        return;
    }

    m_buffer.insert(m_buffer.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    stbtt_InitFont(&m_stbFont, m_buffer.data(), 0);

    //----- Texture size

    m_glyphsScale = stbtt_ScaleForPixelHeight(&m_stbFont, size);
    stbtt_GetFontVMetrics(&m_stbFont, &m_glyphsAscent, 0, 0);
    m_glyphsAscent = m_glyphsAscent * m_glyphsScale;

    int x0, y0, x1, y1;
    stbtt_GetFontBoundingBox(&m_stbFont, &x0, &y0, &x1, &y1);
    m_glyphMaxWidth = std::ceil((x1 - x0) * m_glyphsScale);
    m_glyphMaxHeight = std::ceil((y1 - y0) * m_glyphsScale);

    // @note Texture size can't be updated dynamically,
    // because meshes could reference wrong UVs.
    m_textureWidth = m_glyphMaxWidth * m_maxRenderedGlyphsCount;
    m_textureHeight = m_glyphMaxHeight;
    m_pixels.resize(m_textureWidth * m_glyphMaxHeight);
}

std::vector<Font::GlyphInfo> Font::glyphsInfos(std::wstring_view u16Text)
{
    std::vector<GlyphInfo> glyphsInfos;

    bool textureChanged = false;
    float advance = 0.f;
    uint32_t i = 0u;
    auto c = u16Text[i];
    while (i < u16Text.size()) {
        auto nextC = u16Text[++i];

        // Add the glyph to the texture if it does not exist yet
        auto pGlyphInfo = m_glyphsInfos.find(c);
        if (pGlyphInfo == m_glyphsInfos.end()) {
            m_glyphsInfos[c] = packGlyph(c);
            pGlyphInfo = m_glyphsInfos.find(c);

            textureChanged = textureChanged || (pGlyphInfo->second.width > 0.f && pGlyphInfo->second.height > 0.f);
        }

        // Push non-empty glyphInfos to the list
        if (pGlyphInfo->second.width > 0.f && pGlyphInfo->second.height > 0.f) {
            auto glyphInfo = pGlyphInfo->second;
            glyphInfo.xOffset += advance;
            glyphsInfos.emplace_back(glyphInfo);
            advance = 0.f;
        }

        auto kernAdvance = stbtt_GetCodepointKernAdvance(&m_stbFont, c, nextC) * m_glyphsScale / float(m_glyphMaxHeight);
        advance += pGlyphInfo->second.advance + kernAdvance;
        c = nextC;
    }

    if (textureChanged) {
        texture().loadFromMemory(m_pixels.data(), m_textureWidth, m_textureHeight, 1u);
    }

    return glyphsInfos;
}

//---- Getters

magma::Texture& Font::texture()
{
    if (m_texture == nullptr) {
        m_texture = &m_engine.scene().make<magma::Texture>();
    }
    return *m_texture;
}

//----- Internal

Font::GlyphInfo Font::packGlyph(wchar_t c)
{
    GlyphInfo glyphInfo;

    // Basic glyph advance
    int advance, lsb;
    int x0, y0, x1, y1;
    stbtt_GetCodepointHMetrics(&m_stbFont, c, &advance, &lsb);
    stbtt_GetCodepointBitmapBox(&m_stbFont, c, m_glyphsScale, m_glyphsScale, &x0, &y0, &x1, &y1);

    glyphInfo.advance = (advance * m_glyphsScale - x0) / float(m_glyphMaxHeight);

    // Glyph is empty
    int width = x1 - x0;
    int height = y1 - y0;
    if (width == 0 || height == 0) {
        return glyphInfo;
    }

    glyphInfo.width = width / float(m_glyphMaxHeight);
    glyphInfo.height = height / float(m_glyphMaxHeight);
    glyphInfo.xOffset = x0 / float(m_glyphMaxHeight);
    glyphInfo.yOffset = (m_glyphsAscent + y0) / float(m_glyphMaxHeight);

    // Draw the bitmap to the texture
    auto glyphStartPosition = m_nextGlyphStartPosition;
    stbtt_MakeCodepointBitmap(&m_stbFont, m_pixels.data() + glyphStartPosition,
                              width, height, m_textureWidth, m_glyphsScale, m_glyphsScale, c);
    m_nextGlyphStartPosition += width + 1;

    // Stores the uv informations.
    glyphInfo.minUv.x = static_cast<float>(glyphStartPosition) / static_cast<float>(m_textureWidth);
    glyphInfo.minUv.y = 0.f;
    glyphInfo.maxUv.x = static_cast<float>(glyphStartPosition + width) / static_cast<float>(m_textureWidth);
    glyphInfo.maxUv.y = static_cast<float>(height) / static_cast<float>(m_textureHeight);
    return glyphInfo;
}
