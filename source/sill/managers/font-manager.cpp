#include <lava/sill/managers/font-manager.hpp>

using namespace lava::sill;
using namespace lava::chamber;

FontManager::FontManager(GameEngine& engine)
    : m_engine(engine)
{
}

void FontManager::registerFont(const std::string& hrid, const std::string& path)
{
    PROFILE_FUNCTION(PROFILER_COLOR_REGISTER);

    logger.info("sill.font-manager") << "Adding font " << hrid << "." << std::endl;
    m_paths[hrid].emplace_back(path);
}

Font& FontManager::get(const std::string& hrid, uint32_t size)
{
    auto id = hrid + ":" + std::to_string(size);

    auto fontIt = m_fonts.find(id);
    if (fontIt == m_fonts.end()) {
        auto font = std::make_unique<Font>(m_engine, m_paths[hrid], size);
        m_fonts.emplace(id, std::move(font));
        fontIt = m_fonts.find(id);
    }

    return *fontIt->second;
}
