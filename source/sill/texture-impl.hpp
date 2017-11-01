#pragma once

#include <lava/sill/texture.hpp>

#include <lava/magma/texture.hpp>
#include <lava/sill/game-engine.hpp>

namespace lava::sill {
    class Texture::Impl {
    public:
        Impl(GameEngine& engine);
        Impl(GameEngine& engine, const std::string& imagePath);
        ~Impl();

        // Texture
        void loadFromMemory(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, uint8_t channels);
        void loadFromFile(const std::string& imagePath);

        // Getters
        magma::Texture& texture() { return *m_texture; }
        const magma::Texture& texture() const { return *m_texture; }

    private:
        // References
        GameEngine::Impl& m_engine;

        // Resources
        magma::Texture* m_texture = nullptr;
    };
}
