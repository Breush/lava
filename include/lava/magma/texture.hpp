#pragma once

#include <array>
#include <cstdint>
#include <lava/core/macros/aft.hpp>
#include <lava/core/filesystem.hpp>
#include <memory>

namespace lava::magma {
    class TextureAft;
    class RenderEngine;
}

namespace lava::magma {
    /**
     * Texture to be used and shared between materials.
     */
    class Texture {
    public:
        Texture(RenderEngine& engine, const std::string& imagePath);
        ~Texture();

        $aft_class(Texture);

        const std::string& name() const { return m_name; }

        RenderEngine& engine() { return m_engine; }
        const RenderEngine& engine() const { return m_engine; }

        size_t hash() const { return m_hash; }
        bool cube() const { return m_cube; }

        /**
         * @name Loaders
         */
        /// @{
        static size_t hash(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels);
        void loadFromMemory(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels);
        void loadFromFile(const std::string& imagePath);

        /// Order or filenames must be right, left, top, bottom, front, back.
        /// @note All cubes are currently required to have 4 channels.
        void loadCubeFromMemory(const std::array<const uint8_t*, 6u>& imagesPixels, uint32_t width, uint32_t height);
        void loadCubeFromFiles(const fs::Path& imagesPath);
        /// @}

    private:
        // ----- References
        RenderEngine& m_engine;
        std::string m_name;

        size_t m_hash = 0u;
        bool m_cube = false;
    };

    using TexturePtr = std::shared_ptr<Texture>;
}
