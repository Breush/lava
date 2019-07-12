#pragma once

#include <lava/magma/aft-infos.hpp>

#include <array>
#include <cstdint>

namespace lava::magma {
    class TextureAft;
    class RenderScene;
}

namespace lava::magma {
    /**
     * Texture to be used and shared between materials.
     */
    class Texture {
    public:
        Texture(RenderScene& scene);
        ~Texture();

        /// Internal implementation
        TextureAft& aft() { return reinterpret_cast<TextureAft&>(m_aft); }
        const TextureAft& aft() const { return reinterpret_cast<const TextureAft&>(m_aft); }

        /**
         * @name Loaders
         */
        /// @{
        void loadFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels);
        void loadFromFile(const std::string& imagePath);

        /// Order or filenames must be right, left, top, bottom, front, back.
        /// @note All cubes are currently required to have 4 channels.
        void loadCubeFromMemory(const std::array<const uint8_t*, 6u>& imagesPixels, uint32_t width, uint32_t height);
        void loadCubeFromFiles(const std::string& imagesPath);
        /// @}

    private:
        uint8_t m_aft[MAGMA_SIZEOF_TextureAft];

        // ----- References
        RenderScene& m_scene;
    };
}
