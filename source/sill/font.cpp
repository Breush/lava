#include "./font.hpp"

#include <fstream>
#include <lava/chamber/logger.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/texture.hpp>

using namespace lava::sill;
using namespace lava::chamber;

Font::Font(GameEngine& engine, const std::string& fontPath)
    : m_engine(engine)
{
    //----- Read file

    std::ifstream file(fontPath, std::ios::binary);

    if (!file.is_open()) {
        logger.warning("sill.font") << "Unable to find file " << fontPath << "." << std::endl;
        return;
    }

    m_buffer.insert(m_buffer.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    stbtt_InitFont(&m_stbFont, m_buffer.data(), 0);

    //----- Texture size

    // @note Texture size can't be updated dynamically,
    // because meshes could reference wrong UVs.
    m_glyphMaxHeight = 32u;             // @todo Should be configurable
    m_glyphMaxWidth = m_glyphMaxHeight; // @todo Use stbtt_GetFontBoundingBox to know horizontal extent
    m_glyphsRatio = static_cast<float>(m_glyphMaxWidth) / static_cast<float>(m_glyphMaxHeight);
    m_glyphsScale = stbtt_ScaleForPixelHeight(&m_stbFont, m_glyphMaxHeight);
    stbtt_GetFontVMetrics(&m_stbFont, &m_glyphsAscent, 0, 0);
    m_glyphsAscent = m_glyphsAscent * m_glyphsScale;

    // @todo Texture size should depend on
    // @todo We could just store one channel!
    m_textureWidth = m_glyphMaxWidth * m_maxRenderedGlyphsCount;
    m_textureHeight = m_glyphMaxHeight;
    m_pixels.resize(m_textureWidth * m_glyphMaxHeight * 4u);
}

std::vector<Font::GlyphInfo> Font::glyphsInfos(std::wstring_view u16Text)
{
    std::vector<GlyphInfo> glyphsInfos;

    bool newGlyphs = false;
    uint32_t i = 0u;
    auto c = u16Text[i];
    while (i < u16Text.size()) {
        auto nextC = u16Text[++i];

        // Add the glyph to the texture if it does not exist yet
        auto pGlyphInfo = m_glyphsInfos.find(c);
        if (pGlyphInfo == m_glyphsInfos.end()) {
            newGlyphs = true;

            // @todo
            // std::cout << "Glyph " << std::hex << c << std::dec << " not registered." << std::endl;
            m_glyphsInfos[c] = packGlyph(c);
            pGlyphInfo = m_glyphsInfos.find(c);
        }

        // Push the glyphInfo to the list
        auto glyphInfo = pGlyphInfo->second;
        glyphInfo.advance += stbtt_GetCodepointKernAdvance(&m_stbFont, c, nextC) * m_glyphsScale;
        glyphsInfos.emplace_back(glyphInfo);

        c = nextC;
    }

    if (newGlyphs) {
        texture().loadFromMemory(m_pixels, m_textureWidth, m_glyphMaxHeight, 4u);
    }

    return glyphsInfos;
}

//---- Getters

Texture& Font::texture()
{
    if (m_texture == nullptr) {
        m_texture = &m_engine.make<Texture>();
    }
    return *m_texture;
}

//----- Internal

Font::GlyphInfo Font::packGlyph(wchar_t c)
{
    GlyphInfo glyphInfo;

    // Basic glyph advance
    int advance, lsb;
    stbtt_GetCodepointHMetrics(&m_stbFont, c, &advance, &lsb);
    glyphInfo.advance = advance * m_glyphsScale / m_glyphMaxWidth;

    // @todo Optim: We could use a pre-allocated memory
    // Or put it in the target vector directly - making a loop afterwards
    int width, height, xOff, yOff;
    // @todo Use SubPixel version?
    auto bitmap = stbtt_GetCodepointBitmap(&m_stbFont, m_glyphsScale, m_glyphsScale, c, &width, &height, &xOff, &yOff);
    yOff += m_glyphsAscent;

    // If glyph does not exist
    // @todo Need a generic tofu
    if (bitmap == nullptr) {
        return glyphInfo;
    }

    // Draw the bitmap to the texture
    auto glyphStartPosition = m_renderedGlyphsCount * m_glyphMaxWidth;
    for (auto i = 0; i < width; ++i) {
        for (auto j = 0; j < height; ++j) {
            auto index = 4u * (glyphStartPosition + (i + xOff) + (j + yOff) * m_textureWidth);
            m_pixels[index] = 255u;
            m_pixels[index + 1] = 255u;
            m_pixels[index + 2] = 255u;
            m_pixels[index + 3] = bitmap[i + j * width];
        }
    }

    // Free the allocated memory
    ++m_renderedGlyphsCount;
    stbtt_FreeBitmap(bitmap, m_stbFont.userdata);

    // Stores the uv informations.
    glyphInfo.minUv.x = static_cast<float>(glyphStartPosition) / static_cast<float>(m_textureWidth);
    glyphInfo.minUv.y = 0;
    glyphInfo.maxUv.x = static_cast<float>(glyphStartPosition + m_glyphMaxWidth) / static_cast<float>(m_textureWidth);
    glyphInfo.maxUv.y = -1; // Font bitmaps are reversed
    return glyphInfo;
}
