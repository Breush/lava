#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace lava::magma {
    class Texture;
}

namespace lava::sill {
    class GameEngine;
}

namespace lava::sill {
    /**
     * Stores a texture to be used by materials.
     */
    class Texture final {
    public:
        Texture(GameEngine& engine);
        Texture(GameEngine& engine, const std::string& imagePath);
        ~Texture();

        magma::Texture& original() { return *m_original; }
        const magma::Texture& original() const { return *m_original; }

        void loadFromMemory(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);
        void loadFromFile(const std::string& imagePath);

    private:
        GameEngine& m_engine;
        magma::Texture* m_original = nullptr;
    };
}
