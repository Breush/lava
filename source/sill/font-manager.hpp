#pragma once

#include "./font.hpp"

namespace lava::sill {
    class GameEngine;
}

namespace lava::sill {
    class FontManager {
    public:
        FontManager(GameEngine& engine);

        // @todo Can't it be a make<Font>(hrid, path) inside game-engine ?
        void registerFont(const std::string& hrid, const std::string& path);

        // Getters
        Font& font(const std::string& hrid, uint32_t size);

    private:
        // References
        GameEngine& m_engine;

        std::unordered_map<std::string, std::unique_ptr<Font>> m_fonts; // Key is <hrid>:<size>
        std::unordered_map<std::string, std::string> m_paths; // Key is <hrid>
    };
}
