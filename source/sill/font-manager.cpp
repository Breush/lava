#include "./font-manager.hpp"

using namespace lava::sill;
using namespace lava::chamber;

FontManager::FontManager(GameEngine& engine)
    : m_engine(engine)
{
}

void FontManager::registerFont(const std::string& hrid, const std::string& fontPath)
{
    logger.info("sill.font-manager") << "Adding font " << hrid << "." << std::endl;

    auto font = std::make_unique<Font>(m_engine, fontPath);
    m_fonts.emplace(hrid, std::move(font));
}
