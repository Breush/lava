#pragma once

#include <cstdint>
#include <lava/magma/texture.hpp>
#include <string>
#include <vector>

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

        magma::Texture& magma() { return *m_magma; }
        const magma::Texture& magma() const { return *m_magma; }

        void loadFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels);
        void loadFromFile(const std::string& imagePath);

    private:
        GameEngine& m_engine;
        magma::Texture* m_magma = nullptr;
    };
}
