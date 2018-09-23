#pragma once

#include "./font.hpp"

namespace lava::sill {
    class GameEngine;
}

namespace lava::sill {
    class FontManager {
    public:
        FontManager(GameEngine& engine);

        // @todo Can't it be a make<Font>(hrid, fontPath) ?
        void registerFont(const std::string& hrid, const std::string& fontPath);

        // Getters
        Font& font(const std::string& hrid) { return *m_fonts.at(hrid); }

    private:
        // References
        GameEngine& m_engine;

        std::unordered_map<std::string, std::unique_ptr<Font>> m_fonts;
    };
}
