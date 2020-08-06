#pragma once

#include <lava/flow/audio-source.hpp>

#include <memory>

namespace lava::flow {
    class AudioEngine;
    class IMusicData;
}

namespace lava::flow {
    class Music : public AudioSource {
    public:
        /// Create a new music.
        Music(AudioEngine& engine, const std::shared_ptr<IMusicData>& musicData);

        ~Music();
    };
}
