#pragma once

#include <lava/flow/music-compression-format.hpp>

#include <string>
#include <vector>

namespace lava::flow {
    /**
     * Interface holding information about a music.
     * This can be shared between different instances of flow::Music.
     *
     * Use flow::FileMusicData to make a basic MusicData.
     * Or inherit from this class to provide your own interface
     * with your custom asset manager.
     */
    class IMusicData {
    public:
        IMusicData() = default;
        virtual ~IMusicData() = default;

        virtual const uint8_t* data() const = 0;
        virtual uint32_t size() const = 0;
        virtual MusicCompressionFormat compressionFormat() const = 0;
    };
}
