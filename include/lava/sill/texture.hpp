#pragma once

#include <cstdint>
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

        /// Loaders
        void loadFromMemory(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);
        void loadFromFile(const std::string& imagePath);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }
        const Impl& impl() const { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
