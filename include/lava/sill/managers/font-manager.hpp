#pragma once

#include <lava/sill/font.hpp>

namespace lava::sill {
    class GameEngine;
}

namespace lava::sill {
    class FontManager {
    public:
        FontManager(GameEngine& engine);

        // Save the font
        void registerFont(const std::string& hrid, const std::string& path);

        // Getters
        Font& get(const std::string& hrid, uint32_t size);

    private:
        // References
        GameEngine& m_engine;

        std::unordered_map<std::string, std::unique_ptr<Font>> m_fonts; // Key is <hrid>:<size>
        std::unordered_map<std::string, std::vector<std::string>> m_paths; // Key is <hrid>
    };
}
