#pragma once

#include <array>
#include <cstdint>
#include <lava/core/macros/aft.hpp>
#include <lava/core/filesystem.hpp>

namespace lava::magma {
    class TextureAft;
    class Scene;
}

namespace lava::magma {
    /**
     * Texture to be used and shared between materials.
     */
    class Texture {
    public:
        Texture(Scene& scene, const std::string& imagePath);
        ~Texture();

        $aft_class(Texture);

        Scene& scene() { return m_scene; }
        const Scene& scene() const { return m_scene; }

        /**
         * @name Loaders
         */
        /// @{
        void loadFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels);
        void loadFromFile(const std::string& imagePath);

        /// Order or filenames must be right, left, top, bottom, front, back.
        /// @note All cubes are currently required to have 4 channels.
        void loadCubeFromMemory(const std::array<const uint8_t*, 6u>& imagesPixels, uint32_t width, uint32_t height);
        void loadCubeFromFiles(const fs::Path& imagesPath);
        /// @}

    private:
        // ----- References
        Scene& m_scene;
    };
}
